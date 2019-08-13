// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import android.text.TextUtils;
import org.junit.runner.RunWith;
import org.junit.Test;
import org.robolectric.annotation.Config;
import org.robolectric.RobolectricTestRunner;
import android.os.Build;

/** Basic smoke test verifying that Robolectric is loaded and mocking out Android APIs. */
@Config(manifest=Config.NONE, sdk=29)
@RunWith(RobolectricTestRunner.class)
public class SmokeTest {
    @Test
    public void androidLibraryLoaded() {
        assertTrue(TextUtils.equals("xyzzy", "xyzzy"));
    }

    @Test
    public void usingSdk29() {
        assertEquals(Build.VERSION_CODES.Q, Build.VERSION.SDK_INT);
    }
}
