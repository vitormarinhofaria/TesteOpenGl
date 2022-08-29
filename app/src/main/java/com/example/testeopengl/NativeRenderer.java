package com.example.testeopengl;

import android.graphics.Bitmap;

import android.opengl.EGL14;
import android.opengl.EGL15;
import android.opengl.EGLContext;
import android.opengl.GLES20;
import android.opengl.GLES30;
import android.opengl.GLES32;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;

public class NativeRenderer implements GLSurfaceView.Renderer {
    public static native void saveMusic(byte[] musicBytes);
    public static native void onSurfaceCreatedNative(GL10 gl, EGLConfig config);
    public static native void onSurfaceChangedNative(GL10 gl, int width, int height);
    public static native void onDrawFrameNative(GLES32 gl);

    public static native void setBasePath(String basePath);
    public static native void setTexture(Bitmap bitmap);
    public static native void setRawTexture(int w, int h, byte[] bytes);
    public static native void takePrint();
    public static native void SetScreenResolution(int width, int height);

    public static void TakePrint(){
        Bitmap.createBitmap(350, 240, Bitmap.Config.RGB_565);
    }
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.2F, 0.1F, 0.245F, 1.0F);
        boolean  gl11 = gl instanceof GL11;
        boolean gles10 = gl instanceof GLES20;
        boolean gles30 = gl instanceof GLES30;
        GLES32 gla = gl instanceof GLES32 ? ((GLES32) gl) : null;
        onSurfaceCreatedNative(gl, config);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        onSurfaceChangedNative(gl, width, height);
        SetScreenResolution(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES32 gl11 = gl instanceof GLES32 ? ((GLES32) gl) : null;
        onDrawFrameNative(gl11);
    }
}
