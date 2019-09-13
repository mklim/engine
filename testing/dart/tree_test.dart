// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:test/test.dart';

/// Manual assert fail to force CI to go red when the tree is closed.
void main() {
  const bool treeOpen = false;
  const String message = 'The tree has been manually closed to prevent PR submissions. Check #tree-status for more information.';

  test('The tree is open', () async {
    expect(treeOpen, isTrue, reason: message);
  });
}
