package com.mrcwoods.memories;

import android.content.Context;
import android.net.Uri;
import android.webkit.MimeTypeMap;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class ApiClient {
    public interface Callback<T> {
        void onSuccess(T value);
        void onError(String message);
    }

    public static class ImagePage {
        public final List<ImageItem> items;
        public final Long nextAfterId;

        public ImagePage(List<ImageItem> items, Long nextAfterId) {
            this.items = items;
            this.nextAfterId = nextAfterId;
        }
    }

    public void health(Callback<Boolean> callback) {
        getJson(AppConfig.MEMORIES_API_BASE + "/health", null, new Callback<JSONObject>() {
            @Override
            public void onSuccess(JSONObject value) {
                callback.onSuccess(value.optBoolean("ok", false));
            }

            @Override
            public void onError(String message) {
                callback.onError(message);
            }
        });
    }

    public void exchangeCode(String code, String codeVerifier, Callback<String> callback) {
        new Thread(() -> {
            try {
                String body = "grant_type=authorization_code"
                        + "&code=" + encode(code)
                        + "&redirect_uri=" + encode(AppConfig.OAUTH_REDIRECT_URI)
                        + "&client_id=" + encode(AppConfig.OAUTH_CLIENT_ID)
                        + "&code_verifier=" + encode(codeVerifier)
                        + "&client_secret=" + encode(AppConfig.OAUTH_CLIENT_SECRET);
                JSONObject json = postForm(AppConfig.OAUTH_TOKEN_URL, body, null);
                String token = json.optString("access_token", "");
                if (token.isEmpty()) {
                    callback.onError(json.optString("error", "未返回访问令牌"));
                    return;
                }
                callback.onSuccess(token);
            } catch (Exception exception) {
                callback.onError(exception.getMessage());
            }
        }).start();
    }

    public void userInfo(String token, Callback<UserSession> callback) {
        getJson(AppConfig.OAUTH_USERINFO_URL, "Bearer " + token, new Callback<JSONObject>() {
            @Override
            public void onSuccess(JSONObject json) {
                UserSession session = new UserSession();
                session.accessToken = token;
                session.sub = json.optString("sub", "");
                session.qq = json.optString("name", "");
                session.username = json.optString("username", session.qq);
                session.tenantName = json.optString("tenant_name", "");
                session.tenantSlug = json.optString("tenant_slug", "");
                callback.onSuccess(session);
            }

            @Override
            public void onError(String message) {
                callback.onError(message);
            }
        });
    }

    public void images(long afterId, Callback<ImagePage> callback) {
        getJson(AppConfig.MEMORIES_API_BASE + "/images?after_id=" + afterId, null, new Callback<JSONObject>() {
            @Override
            public void onSuccess(JSONObject json) {
                JSONArray array = json.optJSONArray("data");
                List<ImageItem> items = new ArrayList<>();
                if (array != null) {
                    for (int index = 0; index < array.length(); index++) {
                        JSONObject item = array.optJSONObject(index);
                        if (item != null) {
                            items.add(new ImageItem(item.optLong("id"), item.optString("url"), item.optLong("uploaded_at")));
                        }
                    }
                }
                Long next = json.isNull("next_after_id") ? null : json.optLong("next_after_id");
                callback.onSuccess(new ImagePage(items, next));
            }

            @Override
            public void onError(String message) {
                callback.onError(message);
            }
        });
    }

    public void createMemoryImage(String url, Callback<ImageItem> callback) {
        new Thread(() -> {
            try {
                JSONObject request = new JSONObject();
                request.put("url", url);
                JSONObject json = postJson(AppConfig.MEMORIES_API_BASE + "/images", request.toString(), null);
                callback.onSuccess(new ImageItem(json.optLong("id"), json.optString("url"), json.optLong("uploaded_at")));
            } catch (Exception exception) {
                callback.onError(exception.getMessage());
            }
        }).start();
    }

    public void uploadToImageHost(Context context, Uri uri, String outputFormat, Callback<String> callback) {
        new Thread(() -> {
            String boundary = "MemoriesBoundary" + System.currentTimeMillis();
            try {
                HttpURLConnection connection = open(AppConfig.IMAGE_HOST_API, "POST", null);
                connection.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);
                connection.setDoOutput(true);
                OutputStream output = connection.getOutputStream();
                writeField(output, boundary, "outputFormat", outputFormat);
                writeField(output, boundary, "storage_destination", AppConfig.DEFAULT_STORAGE_DESTINATION);
                if (!"default".equals(AppConfig.DEFAULT_CDN_DOMAIN)) {
                    writeField(output, boundary, "cdn_domain", AppConfig.DEFAULT_CDN_DOMAIN);
                }
                writeFile(context, output, boundary, uri);
                output.write(("--" + boundary + "--\r\n").getBytes(StandardCharsets.UTF_8));
                output.flush();
                JSONObject json = readResponse(connection);
                if (!json.optBoolean("success", false)) {
                    callback.onError(json.optString("message", "图床上传失败"));
                    return;
                }
                callback.onSuccess(json.optString("url", json.optJSONObject("data") == null ? "" : json.optJSONObject("data").optString("url", "")));
            } catch (Exception exception) {
                callback.onError(exception.getMessage());
            }
        }).start();
    }

    public void queryImageHost(String query, Callback<JSONObject> callback) {
        getJson(AppConfig.IMAGE_HOST_API + "?q=" + encode(query), null, callback);
    }

    private void getJson(String url, String auth, Callback<JSONObject> callback) {
        new Thread(() -> {
            try {
                callback.onSuccess(readResponse(open(url, "GET", auth)));
            } catch (Exception exception) {
                callback.onError(exception.getMessage());
            }
        }).start();
    }

    private JSONObject postJson(String url, String body, String auth) throws Exception {
        HttpURLConnection connection = open(url, "POST", auth);
        connection.setRequestProperty("Content-Type", "application/json; charset=utf-8");
        connection.setDoOutput(true);
        connection.getOutputStream().write(body.getBytes(StandardCharsets.UTF_8));
        return readResponse(connection);
    }

    private JSONObject postForm(String url, String body, String auth) throws Exception {
        HttpURLConnection connection = open(url, "POST", auth);
        connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
        connection.setDoOutput(true);
        connection.getOutputStream().write(body.getBytes(StandardCharsets.UTF_8));
        return readResponse(connection);
    }

    private HttpURLConnection open(String url, String method, String auth) throws Exception {
        HttpURLConnection connection = (HttpURLConnection) new URL(url).openConnection();
        connection.setRequestMethod(method);
        connection.setConnectTimeout(15000);
        connection.setReadTimeout(30000);
        connection.setRequestProperty("Accept", "application/json");
        if (auth != null) {
            connection.setRequestProperty("Authorization", auth);
        }
        return connection;
    }

    private JSONObject readResponse(HttpURLConnection connection) throws Exception {
        int status = connection.getResponseCode();
        InputStream input = status >= 200 && status < 300 ? connection.getInputStream() : connection.getErrorStream();
        String body = readString(input);
        JSONObject json = body.isEmpty() ? new JSONObject() : new JSONObject(body);
        if (status < 200 || status >= 300) {
            throw new IllegalStateException(json.optString("message", "HTTP " + status));
        }
        return json;
    }

    private String readString(InputStream input) throws Exception {
        if (input == null) {
            return "";
        }
        BufferedReader reader = new BufferedReader(new InputStreamReader(input, StandardCharsets.UTF_8));
        StringBuilder builder = new StringBuilder();
        String line;
        while ((line = reader.readLine()) != null) {
            builder.append(line);
        }
        return builder.toString();
    }

    private void writeField(OutputStream output, String boundary, String name, String value) throws Exception {
        output.write(("--" + boundary + "\r\n").getBytes(StandardCharsets.UTF_8));
        output.write(("Content-Disposition: form-data; name=\"" + name + "\"\r\n\r\n").getBytes(StandardCharsets.UTF_8));
        output.write((value + "\r\n").getBytes(StandardCharsets.UTF_8));
    }

    private void writeFile(Context context, OutputStream output, String boundary, Uri uri) throws Exception {
        String mime = context.getContentResolver().getType(uri);
        String extension = mime == null ? "jpg" : MimeTypeMap.getSingleton().getExtensionFromMimeType(mime);
        String filename = "memory_upload." + (extension == null ? "jpg" : extension);
        output.write(("--" + boundary + "\r\n").getBytes(StandardCharsets.UTF_8));
        output.write(("Content-Disposition: form-data; name=\"image\"; filename=\"" + filename + "\"\r\n").getBytes(StandardCharsets.UTF_8));
        output.write(("Content-Type: " + (mime == null ? "image/jpeg" : mime) + "\r\n\r\n").getBytes(StandardCharsets.UTF_8));
        InputStream input = new BufferedInputStream(context.getContentResolver().openInputStream(uri));
        byte[] buffer = new byte[8192];
        int read;
        while ((read = input.read(buffer)) != -1) {
            output.write(buffer, 0, read);
        }
        input.close();
        output.write("\r\n".getBytes(StandardCharsets.UTF_8));
    }

    private String encode(String value) {
        try {
            return URLEncoder.encode(value, "UTF-8");
        } catch (Exception ignored) {
            return value;
        }
    }
}