package com.tangjn.examples.libtranscode;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AppOpsManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Binder;
import android.os.Build;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;

import java.lang.reflect.Method;

public class PermissionManager {
    public static final String TAG = "mgdvr";
    public static final int REQ_CODE_STORAGE = 21;
    public static final int REQ_CODE_CAMERA = 22;
    public static final int REQ_CODE_MIC = 23;
    public static final int REQ_CODE_LOCATION = 24;
    public static final int REQ_CODE_BOOT = 25;
    public static final int REQ_CODE_FLOATWINDOW = 26;
    public static final int REQ_CODE_WAKELOCK = 27;

    static final String[] CAMERA = new String[]{
            Manifest.permission.CAMERA
    };

    static final String[] STORAGE = new String[]{
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    static final String[] MIC = new String[]{
            Manifest.permission.RECORD_AUDIO
    };

    static final String[] LOCATION = new String[]{
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_COARSE_LOCATION
    };

    static final String[] FLOATWINDOW = new String[]{
            Manifest.permission.SYSTEM_ALERT_WINDOW
    };

    static final String[] BOOT = new String[]{
            Manifest.permission.RECEIVE_BOOT_COMPLETED
    };

    static final String[] WAKELOCK = new String[]{
            Manifest.permission.WAKE_LOCK
    };

    public static boolean checkGrantPermission(Activity activity, Context context, boolean isRequest){
        try{
            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.READ_EXTERNAL_STORAGE)){
                    Log.d(TAG,"READ_EXTERNAL_STORAGE permission not grant");
                    if(isRequest) {
                        ActivityCompat.requestPermissions(activity, STORAGE, REQ_CODE_STORAGE);
                    }
                    return false;
                } else if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE)){
                    Log.d(TAG,"WRITE_EXTERNAL_STORAGE permission not grant");
                    if(isRequest){
                        ActivityCompat.requestPermissions(activity,STORAGE,REQ_CODE_STORAGE);
                    }
                    return false;
                } else if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.CAMERA)){
                    Log.d(TAG,"CAMERA permission not grant");
                    if(isRequest) {
                        ActivityCompat.requestPermissions(activity, CAMERA, REQ_CODE_CAMERA);
                    }
                    return false;
                } else if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.RECORD_AUDIO)){
                    Log.d(TAG,"RECORD_AUDIO permission not grant");
                    if(isRequest) {
                        ActivityCompat.requestPermissions(activity, MIC, REQ_CODE_MIC);
                    }
                    return false;
                } else if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION)){
                    Log.d(TAG,"ACCESS_FINE_LOCATION permission not grant");
                    if(isRequest) {
                        ActivityCompat.requestPermissions(activity, LOCATION, REQ_CODE_LOCATION);
                    }
                    return false;
                } else if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.ACCESS_COARSE_LOCATION)){
                    Log.d(TAG,"ACCESS_COARSE_LOCATION permission not grant");
                    if(isRequest) {
                        ActivityCompat.requestPermissions(activity, LOCATION, REQ_CODE_LOCATION);
                    }
                    return false;
                }else if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.RECEIVE_BOOT_COMPLETED)){
                    Log.d(TAG,"RECEIVE_BOOT_COMPLETED permission not grant");
                    if(isRequest) {
                        ActivityCompat.requestPermissions(activity, BOOT, REQ_CODE_BOOT);
                    }
                    return false;
                }else if(PackageManager.PERMISSION_DENIED == ContextCompat
                        .checkSelfPermission(context, Manifest.permission.WAKE_LOCK)){
                    Log.d(TAG,"wakelock permission not grant");
                    if(isRequest) {
                        ActivityCompat.requestPermissions(activity, WAKELOCK, REQ_CODE_WAKELOCK);
                    }
                    return false;
                }
                /*****
                 * 判断浮窗权限比较特殊
                 */
                else if(!isGrantFloatWindowPermission(context)){
                    Log.d(TAG,"float window permission not grant");
                    if(isRequest) {
                        showFloatRequestDialog(activity, context, isRequest);
                    }
                    return false;
                }
            }
        }catch (Exception e){
            Log.d(TAG,"grant permission catch exception:"+ Log.getStackTraceString(e));
        }
        return true;
    }


    /**
     * 是否有悬浮窗权限
     */
    public static boolean isRequestFloatPermission(Activity activity) {
        // 如果大于23则直接通过系统判断
        if (Build.VERSION.SDK_INT >= 23) {
            return Settings.canDrawOverlays(activity);
        }
        if (Build.VERSION.SDK_INT >= 19) {
            return checkLowVersion( activity, 24);
        }
        return true;
    }

    @TargetApi(19)
    private static boolean checkLowVersion(Context context, int i) {
        boolean z = false;
        if (Build.VERSION.SDK_INT >= 19) {
            AppOpsManager appOpsManager = (AppOpsManager) context.getSystemService(Context.APP_OPS_SERVICE);
            try {
                Class.forName(appOpsManager.getClass().getName());
                if ((Integer) appOpsManager.getClass()
                        .getDeclaredMethod("checkOp", new Class[]{Integer.TYPE, Integer.TYPE, String.class})
                        .invoke(appOpsManager, new Object[]{Integer.valueOf(i), Integer.valueOf(Binder.getCallingUid()), context.getPackageName()})
                        == 0)
                {
                    z = true;
                }
                return z;
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            return false;
        }

        return false;
    }

    /**
     * 跳转到系统设置界面
     * @param activity activity
     */
    @TargetApi(23)
    private static void toSettingFloatPermission(Activity activity) {
        Intent intent = new Intent();
        intent.setAction("android.settings.action.MANAGE_OVERLAY_PERMISSION");
        String sb = "package:" + activity.getPackageName();
        intent.setData(Uri.parse(sb));
        activity.startActivityForResult(intent, 899);
    }

    /**
     * 判断 悬浮窗口权限是否打开
     *
     * @param context
     * @return true 允许 false禁止
     */
    public static boolean isGrantFloatWindowPermission(Context context) {
        try {
            Object object = context.getSystemService(Context.APP_OPS_SERVICE);
            if (object == null) {
                return false;
            }
            Class localClass = object.getClass();
            Class[] arrayOfClass = new Class[3];
            arrayOfClass[0] = Integer.TYPE;
            arrayOfClass[1] = Integer.TYPE;
            arrayOfClass[2] = String.class;
            Method method = localClass.getMethod("checkOp", arrayOfClass);
            if (method == null) {
                return false;
            }
            Object[] arrayOfObject1 = new Object[3];
            arrayOfObject1[0] = Integer.valueOf(24);
            arrayOfObject1[1] = Integer.valueOf(Binder.getCallingUid());
            arrayOfObject1[2] = context.getPackageName();
            int m = ((Integer) method.invoke(object, arrayOfObject1)).intValue();
            return m == AppOpsManager.MODE_ALLOWED;
        } catch (Exception ex) {

        }
        return false;
    }

    private static void showFloatRequestDialog(final Activity activity, final Context context, final boolean isRequest) {
        AlertDialog create = new AlertDialog.Builder(activity).setTitle("DVR需要浮窗权限")
                .setMessage("请点击“去设置”前往系统设置界面打开浮窗权限")
                .setNegativeButton("取消", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialogInterface, int i) {
                checkGrantPermission(activity,context,isRequest);
            }
        }).setPositiveButton("去设置", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialogInterface, int i) {
                toSettingFloatPermission(activity);
            }
        }).create();
        create.setCanceledOnTouchOutside(false);
        create.show();
    }
}
