package com.mrcwoods.memories;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Typeface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.InputType;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.GridLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class MainActivity extends Activity {
    private static final int REQUEST_PERMISSIONS = 10;
    private static final int REQUEST_PICK_IMAGES = 11;

    private final Handler mainHandler = new Handler(Looper.getMainLooper());
    private final ApiClient api = new ApiClient();
    private final RateLimitedQueue galleryQueue = new RateLimitedQueue();
    private final RateLimitedQueue uploadQueue = new RateLimitedQueue();
    private final List<ImageItem> galleryItems = new ArrayList<>();
    private final List<UploadTask> uploadTasks = new ArrayList<>();

    private LocalStore store;
    private ImageCache imageCache;
    private ThemeConfig theme;
    private UserSession session;
    private LinearLayout root;
    private FrameLayout content;
    private TextView title;
    private long nextAfterId = 0;
    private boolean galleryLoading;
    private String selectedOutputFormat = AppConfig.DEFAULT_OUTPUT_FORMAT;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        AppConfig.load(this);
        store = new LocalStore(this);
        imageCache = new ImageCache(this);
        theme = ThemeConfig.load(store.prefs());
        session = store.loadSession();
        requestInitialPermissions();
        showSplash();
        checkHealth();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_PICK_IMAGES && resultCode == RESULT_OK && data != null) {
            addPickedImages(data);
            showUpload();
        }
    }

    private void requestInitialPermissions() {
        if (Build.VERSION.SDK_INT < 23) {
            return;
        }
        List<String> permissions = new ArrayList<>();
        if (checkSelfPermission(Manifest.permission.ACCESS_NETWORK_STATE) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(Manifest.permission.ACCESS_NETWORK_STATE);
        }
        if (Build.VERSION.SDK_INT >= 33) {
            if (checkSelfPermission(Manifest.permission.READ_MEDIA_IMAGES) != PackageManager.PERMISSION_GRANTED) {
                permissions.add(Manifest.permission.READ_MEDIA_IMAGES);
            }
            if (checkSelfPermission(Manifest.permission.READ_MEDIA_VIDEO) != PackageManager.PERMISSION_GRANTED) {
                permissions.add(Manifest.permission.READ_MEDIA_VIDEO);
            }
        } else if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        }
        if (!permissions.isEmpty()) {
            requestPermissions(permissions.toArray(new String[0]), REQUEST_PERMISSIONS);
        }
    }

    private void showSplash() {
        root = vertical();
        root.setGravity(Gravity.CENTER);
        root.setPadding(dp(24), dp(24), dp(24), dp(24));
        applyBackground(root);
        ImageView icon = new ImageView(this);
        icon.setImageResource(getResources().getIdentifier("ic_app", "drawable", getPackageName()));
        root.addView(icon, new LinearLayout.LayoutParams(dp(92), dp(92)));
        TextView name = text(AppConfig.APP_NAME, 30, true);
        name.setGravity(Gravity.CENTER);
        root.addView(name, matchWrap());
        TextView status = text("正在检查服务状态", 15, false);
        status.setGravity(Gravity.CENTER);
        root.addView(status, matchWrap());
        ProgressBar progress = new ProgressBar(this);
        root.addView(progress, new LinearLayout.LayoutParams(dp(48), dp(48)));
        setContentView(root);
    }

    private void checkHealth() {
        if (!hasNetwork()) {
            showBlocked("网络不可用", "请连接网络后重试。", this::checkHealth);
            return;
        }
        api.health(uiCallback(ok -> {
            if (ok) {
                if (session.isLoggedIn()) {
                    showShell(0);
                } else {
                    showLogin();
                }
            } else {
                showBlocked("服务未就绪", "健康检查没有返回 ok=true。", this::checkHealth);
            }
        }, message -> showBlocked("健康检查失败", message, this::checkHealth)));
    }

    private void showBlocked(String heading, String message, Runnable retry) {
        root = vertical();
        root.setGravity(Gravity.CENTER);
        root.setPadding(dp(28), dp(28), dp(28), dp(28));
        applyBackground(root);
        TextView head = text(heading, 24, true);
        head.setGravity(Gravity.CENTER);
        root.addView(head, matchWrap());
        TextView body = text(message, 15, false);
        body.setGravity(Gravity.CENTER);
        root.addView(body, matchWrap());
        root.addView(button("重试", "ic_link", view -> retry.run()), matchWrap());
        setContentView(root);
    }

    private void showLogin() {
        root = vertical();
        root.setGravity(Gravity.CENTER);
        root.setPadding(dp(28), dp(28), dp(28), dp(28));
        applyBackground(root);
        TextView head = text("登录 Memories", 28, true);
        head.setGravity(Gravity.CENTER);
        root.addView(head, matchWrap());
        TextView body = text("使用校园墙 OAuth 获取个人资料并在本机保存登录状态。", 15, false);
        body.setGravity(Gravity.CENTER);
        root.addView(body, matchWrap());
        root.addView(button("校园墙登录", "ic_user", view -> startOAuth()), matchWrap());
        setContentView(root);
    }

    private void startOAuth() {
        if (!AppConfig.isOAuthConfigured()) {
            new AlertDialog.Builder(this)
                    .setTitle("OAuth 未配置")
                    .setMessage("请先复制 app-config.example.properties 为 app/src/main/assets/app-config.properties，并填写 OAuth 与 API 配置。")
                    .setPositiveButton("确定", null)
                    .show();
            return;
        }
        String state = UUID.randomUUID().toString();
        listenForOAuth(state);
        Uri uri = Uri.parse(AppConfig.OAUTH_AUTHORIZE_URL).buildUpon()
                .appendQueryParameter("response_type", "code")
                .appendQueryParameter("client_id", AppConfig.OAUTH_CLIENT_ID)
                .appendQueryParameter("redirect_uri", AppConfig.OAUTH_REDIRECT_URI)
                .appendQueryParameter("scope", AppConfig.OAUTH_SCOPE)
                .appendQueryParameter("state", state)
                .build();
        startActivity(new Intent(Intent.ACTION_VIEW, uri));
        toast("请在浏览器完成授权");
    }

    private void listenForOAuth(String expectedState) {
        new Thread(() -> {
            try (ServerSocket server = new ServerSocket(AppConfig.OAUTH_CALLBACK_PORT)) {
                Socket socket = server.accept();
                BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                String line = reader.readLine();
                String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n授权完成，请返回 Memories。";
                socket.getOutputStream().write(response.getBytes());
                socket.close();
                String path = line == null ? "" : line.split(" ")[1];
                String query = path.contains("?") ? path.substring(path.indexOf('?') + 1) : "";
                String code = queryValue(query, "code");
                String state = queryValue(query, "state");
                if (code.isEmpty() || !expectedState.equals(state)) {
                    mainHandler.post(() -> toast("OAuth 回调无效"));
                    return;
                }
                api.exchangeCode(code, uiCallback(token -> api.userInfo(token, uiCallback(user -> {
                    session = user;
                    store.saveSession(user);
                    showShell(0);
                }, this::toast)), this::toast));
            } catch (Exception exception) {
                mainHandler.post(() -> toast("无法监听 localhost:2580，可能端口被占用"));
            }
        }).start();
    }

    private void showShell(int selectedTab) {
        root = vertical();
        applyBackground(root);
        LinearLayout header = horizontal();
        header.setGravity(Gravity.CENTER_VERTICAL);
        header.setPadding(dp(18), dp(14), dp(18), dp(8));
        title = text(tabTitle(selectedTab), 22, true);
        header.addView(title, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        root.addView(header, matchWrap());
        content = new FrameLayout(this);
        root.addView(content, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1));
        LinearLayout nav = horizontal();
        nav.setGravity(Gravity.CENTER);
        nav.setPadding(dp(8), dp(8), dp(8), dp(10));
        nav.addView(navButton("广场", "ic_gallery", selectedTab == 0, view -> showGallery()), new LinearLayout.LayoutParams(0, dp(58), 1));
        nav.addView(navButton("上传", "ic_upload", selectedTab == 1, view -> showUpload()), new LinearLayout.LayoutParams(0, dp(58), 1));
        nav.addView(navButton("个人", "ic_user", selectedTab == 2, view -> showProfile()), new LinearLayout.LayoutParams(0, dp(58), 1));
        root.addView(nav, matchWrap());
        setContentView(root);
        if (selectedTab == 0) {
            showGallery();
        } else if (selectedTab == 1) {
            showUpload();
        } else {
            showProfile();
        }
    }

    private void showGallery() {
        title.setText("广场");
        content.removeAllViews();
        ScrollView scroll = new ScrollView(this);
        LinearLayout page = vertical();
        page.setPadding(dp(14), dp(4), dp(14), dp(16));
        TextView hint = text("每 5 秒最多加载 5 张，点击图片可放大或查询图床信息。", 13, false);
        page.addView(hint, matchWrap());
        GridLayout grid = new GridLayout(this);
        grid.setColumnCount(2);
        page.addView(grid, matchWrap());
        Button more = button("加载更多", "ic_gallery", view -> loadGallery(grid));
        page.addView(more, matchWrap());
        scroll.addView(page);
        content.addView(scroll);
        if (galleryItems.isEmpty()) {
            galleryItems.addAll(store.loadImageUrlCache());
            for (ImageItem item : galleryItems) {
                queueImageCard(grid, item);
            }
            loadGallery(grid);
        } else {
            for (ImageItem item : galleryItems) {
                queueImageCard(grid, item);
            }
        }
    }

    private void loadGallery(GridLayout grid) {
        if (galleryLoading) {
            return;
        }
        galleryLoading = true;
        api.images(nextAfterId, uiCallback(page -> {
            nextAfterId = page.nextAfterId == null ? nextAfterId : page.nextAfterId;
            galleryItems.addAll(page.items);
            store.saveImageUrlCache(galleryItems);
            for (ImageItem item : page.items) {
                queueImageCard(grid, item);
            }
            galleryLoading = false;
        }, message -> {
            galleryLoading = false;
            toast(message);
        }));
    }

    private void queueImageCard(GridLayout grid, ImageItem item) {
        galleryQueue.add(() -> imageCache.load(item.url, bitmapCallback(bitmap -> addImageCard(grid, item, bitmap), this::toast)));
    }

    private void addImageCard(GridLayout grid, ImageItem item, Bitmap bitmap) {
        ImageView image = new ImageView(this);
        image.setImageBitmap(bitmap);
        image.setScaleType(ImageView.ScaleType.CENTER_CROP);
        image.setBackgroundColor(adjust(theme.secondaryButton));
        image.setOnClickListener(view -> showImageDialog(item, bitmap));
        GridLayout.LayoutParams params = new GridLayout.LayoutParams();
        params.width = (getResources().getDisplayMetrics().widthPixels - dp(44)) / 2;
        params.height = params.width;
        params.setMargins(dp(4), dp(4), dp(4), dp(4));
        grid.addView(image, params);
    }

    private void showImageDialog(ImageItem item, Bitmap bitmap) {
        LinearLayout layout = vertical();
        layout.setPadding(dp(16), dp(16), dp(16), dp(8));
        ImageView image = new ImageView(this);
        image.setImageBitmap(bitmap);
        image.setAdjustViewBounds(true);
        layout.addView(image, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, dp(360)));
        layout.addView(button("查询图床信息", "ic_link", view -> queryImageInfo(item.url)), matchWrap());
        new AlertDialog.Builder(this).setView(layout).setPositiveButton("关闭", null).show();
    }

    private void queryImageInfo(String url) {
        String q = url.substring(url.lastIndexOf('/') + 1);
        api.queryImageHost(q, uiCallback(json -> {
            JSONObject data = json.optJSONObject("data");
            String message = data == null ? json.toString() : "文件名：" + data.optString("filename")
                    + "\n存储：" + data.optString("storage_location")
                    + "\n标签：" + data.optString("tags")
                    + "\n描述：" + data.optString("content_description");
            new AlertDialog.Builder(this).setTitle("图片信息").setMessage(message).setPositiveButton("确定", null).show();
        }, this::toast));
    }

    private void showUpload() {
        title.setText("上传");
        content.removeAllViews();
        ScrollView scroll = new ScrollView(this);
        LinearLayout page = vertical();
        page.setPadding(dp(16), dp(4), dp(16), dp(16));
        TextView detail = text("存储位置固定 Telegram，CDN 自动选择；队列每 5 秒最多上传 5 张。", 13, false);
        page.addView(detail, matchWrap());
        Spinner format = new Spinner(this);
        String[] formats = {"auto", "jpg", "png", "webp", "gif", "webp_animated"};
        format.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, formats));
        format.setSelection(Math.max(0, java.util.Arrays.asList(formats).indexOf(selectedOutputFormat)));
        format.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                selectedOutputFormat = formats[position];
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
        page.addView(format, matchWrap());
        page.addView(button("选择图片", "ic_upload", view -> pickImages()), matchWrap());
        LinearLayout queueView = vertical();
        page.addView(queueView, matchWrap());
        renderUploadQueue(queueView);
        scroll.addView(page);
        content.addView(scroll);
    }

    private void pickImages() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("image/*");
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent, REQUEST_PICK_IMAGES);
    }

    private void addPickedImages(Intent data) {
        ClipData clip = data.getClipData();
        if (clip != null) {
            for (int index = 0; index < clip.getItemCount(); index++) {
                enqueueUpload(new UploadTask(clip.getItemAt(index).getUri()));
            }
        } else if (data.getData() != null) {
            enqueueUpload(new UploadTask(data.getData()));
        }
    }

    private void enqueueUpload(UploadTask task) {
        uploadTasks.add(task);
        uploadQueue.add(() -> runUpload(task));
    }

    private void runUpload(UploadTask task) {
        task.status = UploadTask.UPLOADING;
        task.progress = 20;
        task.message = "上传到图床";
        mainHandler.post(() -> showUpload());
        api.uploadToImageHost(this, task.uri, selectedOutputFormat, uiCallback(url -> {
            task.progress = 70;
            task.message = "写入 Memories API";
            api.createMemoryImage(url, uiCallback(item -> {
                task.status = UploadTask.DONE;
                task.progress = 100;
                task.url = item.url;
                task.message = "完成";
                store.appendUploadRecord(item.url);
                galleryItems.add(0, item);
                store.saveImageUrlCache(galleryItems);
                showUpload();
            }, message -> failTask(task, message)));
        }, message -> failTask(task, message)));
    }

    private void failTask(UploadTask task, String message) {
        task.status = UploadTask.FAILED;
        task.message = message;
        showUpload();
    }

    private void renderUploadQueue(LinearLayout queueView) {
        queueView.removeAllViews();
        if (uploadTasks.isEmpty()) {
            TextView empty = text("暂无上传任务", 14, false);
            empty.setGravity(Gravity.CENTER);
            queueView.addView(empty, matchWrap());
            return;
        }
        for (UploadTask task : uploadTasks) {
            LinearLayout row = vertical();
            row.setPadding(dp(12), dp(12), dp(12), dp(12));
            row.setBackgroundColor(adjust(theme.secondaryButton));
            row.addView(text(task.uri.getLastPathSegment(), 14, true), matchWrap());
            row.addView(text(task.message, 12, false), matchWrap());
            ProgressBar bar = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
            bar.setMax(100);
            bar.setProgress(task.progress);
            row.addView(bar, matchWrap());
            LinearLayout.LayoutParams params = matchWrap();
            params.setMargins(0, dp(8), 0, dp(8));
            queueView.addView(row, params);
        }
    }

    private void showProfile() {
        title.setText("个人");
        content.removeAllViews();
        ScrollView scroll = new ScrollView(this);
        LinearLayout page = vertical();
        page.setPadding(dp(16), dp(4), dp(16), dp(20));
        page.addView(profileHeader(), matchWrap());
        page.addView(sectionTitle("协议"), matchWrap());
        page.addView(button("隐私协议", "ic_link", view -> showPolicy("隐私协议", "登录信息、QQ号、用户名、图片URL缓存和上传记录仅保存在本机，用于维持登录、加速浏览和展示上传进度。")), matchWrap());
        page.addView(button("服务条款", "ic_link", view -> showPolicy("服务条款", "使用本应用上传图片即表示你确认拥有上传权限，并同意调用 Memories API 与失控图床公开接口。")), matchWrap());
        page.addView(sectionTitle("关于"), matchWrap());
        page.addView(button("查看关于", "ic_app", view -> showAbout()), matchWrap());
        page.addView(sectionTitle("存储管理"), matchWrap());
        storageRows(page);
        page.addView(sectionTitle("主题配置"), matchWrap());
        themeControls(page);
        page.addView(button("退出登录", "ic_clear", view -> {
            store.clearSession();
            session = new UserSession();
            showLogin();
        }), matchWrap());
        scroll.addView(page);
        content.addView(scroll);
    }

    private LinearLayout profileHeader() {
        LinearLayout header = horizontal();
        header.setGravity(Gravity.CENTER_VERTICAL);
        ImageView avatar = new ImageView(this);
        avatar.setBackgroundColor(theme.secondaryButton);
        header.addView(avatar, new LinearLayout.LayoutParams(dp(64), dp(64)));
        imageCache.load("https://q1.qlogo.cn/g?b=qq&nk=" + session.qq + "&s=100", bitmapCallback(avatar::setImageBitmap, message -> {}));
        LinearLayout names = vertical();
        names.setPadding(dp(12), 0, 0, 0);
        names.addView(text(session.username.isEmpty() ? "未命名用户" : session.username, 20, true), matchWrap());
        names.addView(text("QQ " + session.qq, 13, false), matchWrap());
        names.addView(text(session.tenantName, 12, false), matchWrap());
        header.addView(names, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        return header;
    }

    private void showPolicy(String heading, String body) {
        new AlertDialog.Builder(this).setTitle(heading).setMessage(body).setPositiveButton("确定", null).show();
    }

    private void showAbout() {
        LinearLayout layout = vertical();
        layout.setPadding(dp(18), dp(18), dp(18), dp(6));
        ImageView avatar = new ImageView(this);
        layout.addView(avatar, new LinearLayout.LayoutParams(dp(88), dp(88)));
        imageCache.load(AppConfig.DEVELOPER_AVATAR_URL, bitmapCallback(avatar::setImageBitmap, message -> {}));
        layout.addView(text("版本：" + getVersionName(), 14, false), matchWrap());
        layout.addView(text("所属学校：" + AppConfig.SCHOOL_NAME, 14, false), matchWrap());
        layout.addView(text("更新日期：" + AppConfig.UPDATE_DATE, 14, false), matchWrap());
        layout.addView(text("开发者：" + AppConfig.DEVELOPER_NAME, 14, false), matchWrap());
        layout.addView(button("打开官网", "ic_link", view -> openWebsite()), matchWrap());
        new AlertDialog.Builder(this).setTitle("关于 Memories").setView(layout).setPositiveButton("关闭", null).show();
    }

    private void openWebsite() {
        ClipboardManager clipboard = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        clipboard.setPrimaryClip(ClipData.newPlainText("Memories 官网", AppConfig.WEBSITE_URL));
        startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(AppConfig.WEBSITE_URL)));
    }

    private void storageRows(LinearLayout page) {
        storageRow(page, "登录数据", store.preferenceBytes("session"), () -> store.clearKey("session"));
        storageRow(page, "图片缓存数据", imageCache.sizeBytes(), () -> imageCache.clear());
        storageRow(page, "图片URL缓存", store.preferenceBytes("image_url_cache"), () -> store.clearKey("image_url_cache"));
        storageRow(page, "上传记录", store.preferenceBytes("upload_records"), () -> store.clearKey("upload_records"));
    }

    private void storageRow(LinearLayout page, String label, long bytes, Runnable clear) {
        LinearLayout row = horizontal();
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.addView(text(label + " · " + formatSize(bytes), 14, false), new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        row.addView(button("清除", "ic_clear", view -> {
            clear.run();
            showProfile();
        }), new LinearLayout.LayoutParams(dp(112), dp(48)));
        page.addView(row, matchWrap());
    }

    private void themeControls(LinearLayout page) {
        page.addView(button(theme.darkMode ? "切换亮色主题" : "切换暗色主题", "ic_link", view -> {
            theme.applyPreset(!theme.darkMode);
            theme.save(store.prefs());
            showShell(2);
        }), matchWrap());
        colorControl(page, "主按钮颜色", theme.primaryButton, value -> theme.primaryButton = value);
        colorControl(page, "次要按钮颜色", theme.secondaryButton, value -> theme.secondaryButton = value);
        colorControl(page, "背景颜色", theme.background, value -> theme.background = value);
        colorControl(page, "主文字颜色", theme.primaryText, value -> theme.primaryText = value);
        colorControl(page, "次要文字颜色", theme.secondaryText, value -> theme.secondaryText = value);
        Spinner fonts = new Spinner(this);
        String[] families = {"sans", "serif", "monospace", "casual"};
        fonts.setAdapter(new ArrayAdapter<>(this, android.R.layout.simple_spinner_dropdown_item, families));
        fonts.setSelection(Math.max(0, java.util.Arrays.asList(families).indexOf(theme.fontFamily)));
        fonts.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                theme.fontFamily = families[position];
                theme.save(store.prefs());
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
        page.addView(fonts, matchWrap());
        TextView sizeLabel = text("字体大小：" + Math.round(theme.fontScale * 100) + "%", 14, false);
        page.addView(sizeLabel, matchWrap());
        SeekBar size = new SeekBar(this);
        size.setMax(80);
        size.setProgress((int) ((theme.fontScale - 0.8f) * 100));
        size.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                theme.fontScale = 0.8f + progress / 100f;
                sizeLabel.setText("字体大小：" + Math.round(theme.fontScale * 100) + "%");
                theme.save(store.prefs());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                showShell(2);
            }
        });
        page.addView(size, matchWrap());
    }

    private void colorControl(LinearLayout page, String label, int current, ColorSetter setter) {
        LinearLayout row = horizontal();
        row.setGravity(Gravity.CENTER_VERTICAL);
        EditText input = new EditText(this);
        input.setSingleLine(true);
        input.setInputType(InputType.TYPE_CLASS_TEXT);
        input.setText(String.format("#%06X", 0xFFFFFF & current));
        row.addView(text(label, 13, false), new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1));
        row.addView(input, new LinearLayout.LayoutParams(dp(120), dp(54)));
        row.addView(button("应用", "ic_link", view -> {
            setter.set(ThemeConfig.parseColorOrDefault(input.getText().toString(), current));
            theme.save(store.prefs());
            showShell(2);
        }), new LinearLayout.LayoutParams(dp(96), dp(48)));
        page.addView(row, matchWrap());
    }

    private Button button(String label, String icon, View.OnClickListener listener) {
        Button button = new Button(this);
        button.setText(label);
        button.setTextColor(theme.primaryText);
        button.setTextSize(14 * theme.fontScale);
        button.setTypeface(Typeface.create(theme.fontFamily, Typeface.NORMAL));
        button.setAllCaps(false);
        button.setCompoundDrawablesWithIntrinsicBounds(getResources().getIdentifier(icon, "drawable", getPackageName()), 0, 0, 0);
        button.setCompoundDrawablePadding(dp(6));
        button.setBackgroundColor(theme.secondaryButton);
        button.setOnClickListener(listener);
        return button;
    }

    private Button navButton(String label, String icon, boolean selected, View.OnClickListener listener) {
        Button button = button(label, icon, listener);
        button.setTextColor(selected ? theme.background : theme.primaryText);
        button.setBackgroundColor(selected ? theme.primaryButton : theme.secondaryButton);
        return button;
    }

    private TextView sectionTitle(String value) {
        TextView text = text(value, 17, true);
        text.setPadding(0, dp(18), 0, dp(6));
        return text;
    }

    private TextView text(String value, int size, boolean bold) {
        TextView text = new TextView(this);
        text.setText(value == null ? "" : value);
        text.setTextColor(bold ? theme.primaryText : theme.secondaryText);
        text.setTextSize(size * theme.fontScale);
        text.setTypeface(Typeface.create(theme.fontFamily, bold ? Typeface.BOLD : Typeface.NORMAL));
        text.setLineSpacing(dp(2), 1.0f);
        text.setPadding(0, dp(4), 0, dp(4));
        return text;
    }

    private LinearLayout vertical() {
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        return layout;
    }

    private LinearLayout horizontal() {
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.HORIZONTAL);
        return layout;
    }

    private LinearLayout.LayoutParams matchWrap() {
        return new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
    }

    private void applyBackground(View view) {
        view.setBackgroundColor(theme.background);
    }

    private int adjust(int color) {
        return Color.argb(255, Color.red(color), Color.green(color), Color.blue(color));
    }

    private <T> ApiClient.Callback<T> uiCallback(UiSuccess<T> success, UiError error) {
        return new ApiClient.Callback<T>() {
            @Override
            public void onSuccess(T value) {
                mainHandler.post(() -> success.run(value));
            }

            @Override
            public void onError(String message) {
                mainHandler.post(() -> error.run(message == null ? "请求失败" : message));
            }
        };
    }

    private ImageCache.BitmapCallback bitmapCallback(UiSuccess<Bitmap> success, UiError error) {
        return new ImageCache.BitmapCallback() {
            @Override
            public void onReady(Bitmap bitmap) {
                mainHandler.post(() -> success.run(bitmap));
            }

            @Override
            public void onError(String message) {
                mainHandler.post(() -> error.run(message == null ? "图片加载失败" : message));
            }
        };
    }

    private boolean hasNetwork() {
        ConnectivityManager manager = (ConnectivityManager) getSystemService(CONNECTIVITY_SERVICE);
        NetworkInfo info = manager == null ? null : manager.getActiveNetworkInfo();
        return info != null && info.isConnected();
    }

    private String queryValue(String query, String key) throws Exception {
        for (String pair : query.split("&")) {
            String[] parts = pair.split("=", 2);
            if (parts.length == 2 && key.equals(parts[0])) {
                return URLDecoder.decode(parts[1], "UTF-8");
            }
        }
        return "";
    }

    private String tabTitle(int tab) {
        if (tab == 1) {
            return "上传";
        }
        if (tab == 2) {
            return "个人";
        }
        return "广场";
    }

    private String getVersionName() {
        try {
            return getPackageManager().getPackageInfo(getPackageName(), 0).versionName;
        } catch (Exception ignored) {
            return "1.0.0";
        }
    }

    private String formatSize(long bytes) {
        if (bytes < 1024) {
            return bytes + " B";
        }
        double kb = bytes / 1024.0;
        if (kb < 1024) {
            return String.format("%.1f KB", kb);
        }
        return String.format("%.1f MB", kb / 1024.0);
    }

    private int dp(int value) {
        return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
    }

    private void toast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    private interface UiSuccess<T> {
        void run(T value);
    }

    private interface UiError {
        void run(String message);
    }

    private interface ColorSetter {
        void set(int color);
    }
}