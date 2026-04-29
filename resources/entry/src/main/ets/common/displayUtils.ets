/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import display from '@ohos.display';
import window from '@ohos.window';
import Constants from './constant';

export default class DisplayUtils {
  private constructor() {
  }

  /**
   * @returns display.Display if success, undefined if failed
   */
  public static getCurWindowDisplay(win: window.Window): display.Display | undefined {
    try {
      let displayId: number | undefined = win.getWindowProperties().displayId;
      console.info(Constants.TAG, `get displayId by window, displayId=${displayId}.`);
      if (displayId !== undefined) {
        let currentDisplay = display.getDisplayByIdSync(displayId);
        console.info(Constants.TAG, `getCurWindowDisplay success.`);
        return currentDisplay;
      }
      return undefined;
    } catch (err) {
      console.error(Constants.TAG, `getCurWindowDisplay failed, code=${err?.code}, msg=${err?.message}.`);
      return undefined;
    }
  }

}