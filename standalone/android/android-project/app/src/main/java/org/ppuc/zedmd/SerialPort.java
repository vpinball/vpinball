package org.ppuc.zedmd;

import com.hoho.android.usbserial.*;
import com.hoho.android.usbserial.driver.*;
import com.hoho.android.usbserial.util.*;
import android.app.Activity;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbDeviceConnection;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.util.List;
import java.util.Queue;
import java.util.ArrayDeque;

public class SerialPort implements SerialInputOutputManager.Listener {
    private static final String TAG = "SerialPort";

    private static SerialPort instance;
    
    private Activity activity;
    private SerialInputOutputManager usbIoManager;
    private UsbSerialPort usbSerialPort;
    private PipedOutputStream outputStream;
    private PipedInputStream inputStream;
    
    private SerialPort() {
    }

    public static SerialPort getInstance() {
        if (instance == null) {
            instance = new SerialPort();
        }
        return instance;
    }

    public void setActivity(Activity activity) {
        this.activity = activity;
    }

    @Override
    public void onNewData(byte[] data) {
        //Log.v(TAG, "onNewData: data=\n" + dumpHexString(data));

        try {
           outputStream.write(data);
        }

        catch(Exception e) {
            Log.v(TAG, "onNewData: exception=" + e.getMessage());
        }
    }

    @Override
    public void onRunError(Exception e) {
        Log.v(TAG, "onRunError: exception=" + e.getMessage());
    }

    public boolean open(int baudRate, int dataBits, int stopBits, int parity) {
        try {
           UsbManager manager = (UsbManager)activity.getSystemService(Context.USB_SERVICE);
           List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);
           if (availableDrivers.isEmpty()) {
              return false;
           }

           UsbSerialDriver driver = availableDrivers.get(0);
           UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
           if (connection == null) {
              return false;
           }

           usbSerialPort = driver.getPorts().get(0);

           Log.v(TAG, "Connecting to " + usbSerialPort.getDevice().getDeviceName());

           usbSerialPort.open(connection);

           outputStream = new PipedOutputStream();
           inputStream = new PipedInputStream(outputStream);

           usbIoManager = new SerialInputOutputManager(usbSerialPort, this);
           usbIoManager.start();
           
           usbSerialPort.setParameters(baudRate, dataBits, stopBits, parity);
           
           return true;
        }

        catch(Exception e) {
            Log.v(TAG, "open: exception=" + e.getMessage());

            usbSerialPort = null;
        }

        return false;
    }

    public boolean isOpen() {
        boolean open;

        try {
           open = usbSerialPort.isOpen();
        }

        catch(Exception e) {
           open = false;
        }

        return open;
    }

    public void close() {
        try {
           usbSerialPort.close();
        }

        catch(Exception e) {
            Log.v(TAG, "close: exception=" + e.getMessage());
        }
    }

    public int available() {
        int i = 0;
        
        try {
           i = inputStream.available();
        }

        catch(Exception e) {
            Log.v(TAG, "available: exception=" + e.getMessage());
        }

        return i;
    }

    public void clearDTR() {
        try {
            usbSerialPort.setDTR(false);
        }

        catch(Exception e) {
            Log.v(TAG, "clearDTR: exception=" + e.getMessage());
        }
    }

    public void setDTR() {
        try {
            usbSerialPort.setDTR(true);
        }

        catch(Exception e) {
            Log.v(TAG, "setDTR: exception=" + e.getMessage());
        }
    }

    public void clearRTS() {
        try {
            usbSerialPort.setRTS(false);
        }

        catch(Exception e) {
            Log.v(TAG, "clearRTS: exception=" + e.getMessage());
        }
    }

    public void setRTS() {
        try {
            usbSerialPort.setRTS(true);
        }

        catch(Exception e) {
            Log.v(TAG, "setRTS: exception=" + e.getMessage());
        }
    }

    public int readBytes(byte[] bytes, int wait) {
        //Log.v(TAG, "readBytes: length=" + bytes.length + ", wait=" + wait );

        int i;

        try {
            long start = System.currentTimeMillis();
            while (inputStream.available() < bytes.length && System.currentTimeMillis() - start < wait) {
                Thread.sleep(100);
            }

            i = inputStream.read(bytes, 0, bytes.length);
            
            //Log.v(TAG, "readBytes: wanted=" + bytes.length + ", got=" + i + ", bytes=\n" + dumpHexString(bytes));
        }

        catch(Exception e) {
            Log.v(TAG, "readBytes: exception=" + e.getMessage());

            i = 0;
        }

        return i;
    }

    public int writeBytes(byte[] bytes, int wait) {
        //Log.v(TAG, "writeBytes: length=" + bytes.length + ", wait=" + wait);

        int i;

        try {        
            //Log.v(TAG, "writeBytes: bytes=\n" + dumpHexString(bytes));

            usbSerialPort.write(bytes, wait);

            i = 1;
        }

        catch(Exception e) {
            Log.v(TAG, "writeBytes: exception=" + e.getMessage());

            i = 0;
        }

        return i;
    }

    private String dumpHexString(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        int i = 0;
        while (i < bytes.length) {
            sb.append(String.format("%04x: ", i));
            int j;
            for (j = 0; j < 8 && i + j < bytes.length; j++) {
                sb.append(String.format("%02x ", bytes[i + j] & 0xff));
            }
            for (; j < 8; j++) {
                sb.append("   ");
            }
            sb.append(" ");
            for (j = 0; j < 8 && i + j < bytes.length; j++) {
                byte b = bytes[i + j];
                if (b >= 32 && b <= 126) {
                    sb.append((char) b);
                } else {
                    sb.append(".");
                }
            }
            sb.append("\n");
            i += 8;
        }
        return sb.toString();
    }
}