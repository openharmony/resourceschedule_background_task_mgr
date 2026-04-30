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

import emitter from '@ohos.events.emitter';
import window from '@ohos.window';
import CommonEventManager from '@ohos.commonEventManager';
import type Want from '@ohos.app.ability.Want';
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import uiExtension from '@ohos.arkui.uiExtension';
import configPolicy from '@ohos.configPolicy';
import fs from '@ohos.file.fs';
import Constants from '../common/constant';
import DisplayUtils from '../common/displayUtils';
import { shouldMoveWindow} from '../common/utils';
import deviceInfo  from '@ohos.deviceInfo';

const TAG = 'BgTaskDialog_Service ';
const KEY_APP_INDEX = 'appIndex';
const KEY_TASK_MODE = 'taskMode';

const UPDATE_INIT = 1;
const UPDATE_NUM = 1;
const UPDATE_BOUNDARY = 100;

let eventSubscriber:CommonEventManager.CommonEventSubscriber;

const enableNotificationDialogDestroyedEvent = {
  eventId: 1,
  priority: emitter.EventPriority.LOW
};

const COMMON_EVENT_NAME = 'OnBgTaskServiceDialogClicked';

// 异常退出。场景：弹窗后，上滑进入多任务，退出应用
async function handleDialogQuitException(want: Want): Promise<void> {
  CommonEventManager.publish(
    COMMON_EVENT_NAME,
    {
      code: 10,
      data: want.parameters.bundleName.toString(),
      parameters: {
        bundleName: want.parameters.bundleName.toString(),
        bundleUid: want.parameters.bundleUid.toString(),
        appIndex: want.parameters.appIndex.toString()
      }
    } as CommonEventManager.CommonEventPublishData,
    () => { console.info(TAG, 'publish DIALOG_CRASHED succeeded'); }
  );
}

export class EnableBgTaskAuthDialog {
  static DIALOG_PATH = 'pages/notificationDialog';
  static PC_DIALOG_PATH = 'pages/pcNotificationDialog';
  static EMPTY_PAGE_PATH = 'pages/emptyPage';
  static MASK_COLOR = '#33000000';
  static TRANSPARANT_COLOR = '#00000000';
  static SCENEBOARD_BUNDLE = 'com.ohos.sceneboard';
  static SYSTEMUI_BUNDLE = 'com.ohos.systemui';

  id: number;
  want: Want;
  window: window.Window;
  extensionWindow:uiExtension.WindowProxy;
  storage: LocalStorage;
  stageModel: boolean;
  subWindow: window.Window;
  initSubWindowSize: boolean;
  innerLake: boolean;
  easyAbroad: boolean;

  constructor(id: number, want: Want, stageModel: boolean, innerLake: boolean, easyAbroad: boolean) {
    this.id = id;
    this.want = want;
    this.stageModel = stageModel;
    this.window = undefined;
    this.extensionWindow = undefined;
    this.initSubWindowSize = false;
    this.innerLake = innerLake;
    this.easyAbroad = easyAbroad;
  }


  async createUiExtensionWindow(session: UIExtensionContentSession, stageModel: boolean): Promise<void> {
    try {
      let extensionWindow = session.getUIExtensionWindowProxy();
      this.extensionWindow = extensionWindow;
      let shouldHide = true;

      this.storage = new LocalStorage({
        'dialog': this,
        'session': session
      });

      let path = EnableBgTaskAuthDialog.DIALOG_PATH;
      let deviceTypeInfo: string = deviceInfo.deviceType;
      console.info(TAG, `current device type : ${deviceTypeInfo}}`);
      if (deviceTypeInfo === '2in1') {
        path = EnableBgTaskAuthDialog.PC_DIALOG_PATH;
      }
      await session.loadContent(path, this.storage);  
        try {    
          await extensionWindow.hideNonSecureWindows(shouldHide);
        } catch (err) {
          console.error(TAG, 'window hideNonSecureWindows failed!');
        }
        await session.setWindowBackgroundColor(EnableBgTaskAuthDialog.TRANSPARANT_COLOR);
    } catch (err) {
      console.error(TAG, 'window create failed!');
      throw new Error('Failed to create window');
    }
  }

  async sleep(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

  async publishButtonClickedEvent(authType: number): Promise<void> {
    CommonEventManager.publish(
      COMMON_EVENT_NAME,
      {
        code: authType,
        data: this.want.parameters.bundleName.toString(),
        parameters: {
          bundleName: this.want.parameters.bundleName.toString(),
          bundleUid: this.want.parameters.bundleUid.toString(),
          appIndex: this.want.parameters.appIndex.toString()
        }
      } as CommonEventManager.CommonEventPublishData,
      () => { console.info(TAG, 'publish CLICKED succeeded'); }
    );
  }

  async destroyException(): Promise<void> {
    await handleDialogQuitException(this.want);
  }

  async destroy(): Promise<void> {
    if (this.window !== undefined) {
      emitter.emit(enableNotificationDialogDestroyedEvent, {
        data: {
          'id': this.id
        }
      });
      await this.destroyWindow();
    }
  }

  async destroyWindow(): Promise<void> {
    await this.window.destroyWindow();
    this.window = undefined;
  }
};


class NotificationDialogServiceExtensionAbility extends UIExtensionAbility {

  onCreate() {
    console.log(TAG, `UIExtAbility onCreate`);
    AppStorage.setOrCreate('context', this.context);
    AppStorage.setOrCreate('isUpdate', UPDATE_INIT);
    AppStorage.setOrCreate('clicked', false);
    this.subscribe();
  
  }

  async onSessionCreate(want: Want, session: UIExtensionContentSession) {
    try {
      let bundleName = want.parameters['ohos.aafwk.param.callerBundleName'];
      let bundleUid = want.parameters['ohos.aafwk.param.callerUid'];
      let appIndex = want.parameters[KEY_APP_INDEX];
      let taskMode = want.parameters[KEY_TASK_MODE];
      want.parameters.bundleName = bundleName;
      want.parameters.bundleUid = bundleUid;
      want.parameters.appIndex = appIndex;
      want.parameters.taskMode = taskMode;
      console.log(TAG, `UIExtAbility onSessionCreate bundleName ${want.parameters.bundleName}` +
        `uid ${want.parameters.bundleUid}` + `appIndex ${want.parameters.appIndex}` + `taskMode ${want.parameters.taskMode}`);    
        let dialog = new EnableBgTaskAuthDialog(1, want, true, false, false);
      await dialog.createUiExtensionWindow(session, true);
      AppStorage.setOrCreate('dialog', dialog);
    } catch (err) {
      console.error(TAG, `Failed to handle onSessionCreate`);
      await handleDialogQuitException(want);
      this.context.terminateSelf();
    }
  }

  onForeground() {
    console.log(TAG, `UIExtAbility onForeground`);
    let dialog = AppStorage.get<EnableBgTaskAuthDialog>('dialog');
    
    if (dialog?.subWindow !== undefined) {
      try {
        dialog?.subWindow?.hideNonSystemFloatingWindows(true);
      } catch (err) {
        console.error(TAG, 'onForeground hideNonSystemFloatingWindows failed!');
      } 
    } else {
      try {
        dialog?.extensionWindow?.hideNonSecureWindows(true);
      } catch (err) {
        console.error(TAG, 'onForeground hideNonSecureWindows failed!');
      }  
    }
  }

  onBackground() {
    console.log(TAG, `UIExtAbility onBackground`);
    let dialog = AppStorage.get<EnableBgTaskAuthDialog>('dialog');

    if (dialog?.subWindow !== undefined) {
      try {
        dialog?.subWindow?.hideNonSystemFloatingWindows(false);
      } catch (err) {
        console.error(TAG, 'onBackground hideNonSystemFloatingWindows failed!');
      } 
    } else {
      try {
        dialog?.extensionWindow?.hideNonSecureWindows(false);
      } catch (err) {
        console.error(TAG, 'onBackground hideNonSecureWindows failed!');
      }  
    }
  }

  async onSessionDestroy(session: UIExtensionContentSession): Promise<void> {
    console.log(TAG, `UIExtAbility onSessionDestroy`);  
    if (AppStorage.get('clicked') === false) {
      console.log(TAG, `UIExtAbility onSessionDestroy unclick destory`);
      let dialog = AppStorage.get<EnableBgTaskAuthDialog>('dialog');
      await dialog?.destroyException();
    }
  }

  async onDestroy(): Promise<void> {
    console.info(TAG, 'UIExtAbility onDestroy.');
    await this.unsubscribe();
    await this.sleep(500);
    this.context.terminateSelf();
  }

  async sleep(ms: number): Promise<void> {
      return new Promise(resolve => setTimeout(resolve, ms));
  }

  async subscribe(): Promise<void> {
    await CommonEventManager.createSubscriber(
      { events: ['usual.event.BUNDLE_RESOURCES_CHANGED'] })
      .then((subscriber:CommonEventManager.CommonEventSubscriber) => {
        eventSubscriber = subscriber;
      })
      .catch((err) => {
        console.log(TAG, `subscriber createSubscriber error code is ${err.code}, message is ${err.message}`);
      });

    if (eventSubscriber === null) {
      console.log(TAG, 'need create subscriber');
      return;
    }
    CommonEventManager.subscribe(eventSubscriber, (err, data) => {
      if (err?.code) {
        console.error(TAG, `subscribe callBack err= ${JSON.stringify(err)}`);
      } else {
        console.log(TAG, `subscribe callBack data= ${JSON.stringify(data)}`);
        if (data.parameters?.bundleResourceChangeType !== 1) {
          return;
        }
        console.log(TAG, `BUNDLE_RESOURCES_CHANGED-language change`);
        let isUpdate:number = AppStorage.get('isUpdate');
        if (isUpdate === undefined || isUpdate > UPDATE_BOUNDARY) {
          AppStorage.setOrCreate('isUpdate', UPDATE_NUM);
        } else {
          AppStorage.setOrCreate('isUpdate', ++isUpdate);
        }
      }
    });
  }

  async unsubscribe(): Promise<void> {
    try {
      if (eventSubscriber != null) {
        CommonEventManager.unsubscribe(eventSubscriber, (err) => {});
      }      
    } catch (err) {
      console.info('ubsubscribe fail');
    }
  }
}

export default NotificationDialogServiceExtensionAbility;