package com.mrcwoods.memories;

import android.net.Uri;

public class UploadTask {
    public static final int WAITING = 0;
    public static final int UPLOADING = 1;
    public static final int DONE = 2;
    public static final int FAILED = 3;

    public final Uri uri;
    public int status = WAITING;
    public float progress;
    public String message = "等待上传";
    public String url = "";

    public UploadTask(Uri uri) {
        this.uri = uri;
    }
}