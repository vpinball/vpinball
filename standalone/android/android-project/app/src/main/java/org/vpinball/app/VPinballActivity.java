package org.vpinball.app;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import org.libsdl.app.SDLActivity;
import org.ppuc.zedmd.SerialPort;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A sample wrapper class that just calls SDLActivity
 */

public class VPinballActivity extends SDLActivity {
    private static final String TAG = "VPinballActivity";

    public void copyAssets(AssetManager assetManager, String srcDir, File dstDir) throws IOException {
        String[] files = assetManager.list(srcDir);
        for (String file : files) {
            String srcPath = "";

            if (!srcDir.isEmpty()) {
                srcPath = srcDir + "/";
            }

            srcPath += file;

            File dstFile = new File(dstDir, file);

            if (dstFile.exists() && !dstFile.isDirectory()) {
                continue;
            }

            try {
                InputStream inputStream = assetManager.open(srcPath);

                Log.v(TAG, "Copying " + srcPath + " to " + dstFile);
                copyFile(inputStream, dstFile);
            }

            catch (FileNotFoundException e) {
                dstFile.mkdirs();
                copyAssets(assetManager, srcPath, dstFile);
            }

            catch (IOException e) {
                Log.v(TAG, "Unable to copy " + srcPath, e);
            }
        }
    }

    public void copyFile(InputStream in, File dstFile) throws IOException {
        OutputStream out = new FileOutputStream(dstFile);
        byte[] buf = new byte[1024];
        int len;
        while ((len = in.read(buf)) > 0) {
            out.write(buf, 0, len);
        }
        in.close();
        out.close();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        SerialPort.getInstance().setActivity(this);

        try {
            copyAssets(this.getAssets(), "", this.getFilesDir());
        }

        catch(Exception e) {
            Log.v(TAG, "Exception while copying assets", e);
        }
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "SDL2_image",
                "FreeImage",
                "bass",
                "pinmame.3.6",
                "zedmd",
                "serum",
                "altsound",
                "vpinball"
        };
    }
}
