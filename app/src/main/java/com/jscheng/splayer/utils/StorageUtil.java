package com.jscheng.splayer.utils;
import android.annotation.SuppressLint;
import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.MediaStore;

import java.io.File;

/**
 * Created By Chengjunsen on 2018/9/5
 */
public class StorageUtil {
    public static String getDirName() {
        return "SPlayer";
    }

    public static String getSDPath() {
        // 判断是否挂载
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            return Environment.getExternalStorageDirectory().getAbsolutePath();
        }
        return Environment.getRootDirectory().getAbsolutePath();
    }

    public static String getImagePath() {
        String path = getSDPath() + "/" + getDirName() + "/image/";
        dirNotExistAndCreate(path);
        return path;
    }

    public static String getVedioPath() {
        String path = getSDPath() + "/" + getDirName() + "/video/";
        dirNotExistAndCreate(path);
        return path;
    }

    public static boolean dirNotExistAndCreate(String path) {
        File mDir = new File(path);
        if (!mDir.exists()) {
            return mDir.mkdirs();
        }
        return true;
    }

    public static boolean isFileExist(String path) {
        File mFile = new File(path);
        return mFile.exists();
    }

    public static boolean deleteFile(String path) {
        File mFile = new File(path);
        if (mFile.exists()) {
            return mFile.delete();
        }
        return true;
    }

    @SuppressLint("NewApi")
    public static String getUrlAbsulotePath(final Context context, final Uri uri) {
        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];
                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }

            }
            else if (isDownloadsDocument(uri)) {
                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));
                return getDataColumn(context, contentUri, null, null);            }

            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];
                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }
                final String selection = "_id=?";
                final String[] selectionArgs = new String[]{split[1]};
                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }
        else if ("content".equalsIgnoreCase(uri.getScheme())) {
            return getDataColumn(context, uri, null, null);
        }
        else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }
        return null;
    }

    /**
     *  * Get the value of the data column for this Uri. This is useful for
     *  * MediaStore Uris, and other file-based ContentProviders.
     *  *
     *  * @param context
     *  The context.
     *  * @param uri
     *  The Uri to query.
     *  * @param selection
     *  (Optional) Filter used in the query.
     *  * @param selectionArgs (Optional) Selection arguments used in the query.
     *  * @return The value of the _data column, which is typically a file path.
     *  */
    public static String getDataColumn(Context context, Uri uri, String selection,
                                       String[] selectionArgs) {
        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = {column};
        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                final int column_index = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(column_index);
            }
        } finally {
            if (cursor != null)
                cursor.close();
        }        return null;
    }

    /**
     *  * @param uri The Uri to check.
     *  * @return Whether the Uri authority is ExternalStorageProvider.
     *  */
    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     *  * @param uri The Uri to check.
     *  * @return Whether the Uri authority is DownloadsProvider.
     *  */
    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     *  * @param uri The Uri to check.
     *  * @return Whether the Uri authority is MediaProvider.
     *  */
    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

}