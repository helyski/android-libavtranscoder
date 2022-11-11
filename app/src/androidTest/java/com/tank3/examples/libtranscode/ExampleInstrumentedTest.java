package com.tank3.examples.libtranscode;

import android.content.Context;
import android.support.test.InstrumentationRegistry;


import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */

public class ExampleInstrumentedTest {
    @Test
    public void useAppContext() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        assertEquals("com.tank3.examples.ffmpeg", appContext.getPackageName());
    }
}