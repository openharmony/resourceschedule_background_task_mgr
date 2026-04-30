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

import bundleResourceManager from '@ohos.bundle.bundleResourceManager';
import display from '@ohos.display';
import window from '@ohos.window';
import {
  titleTrim, 
  calContainerWidth,
  getFontSizeScale,
  sourceToVp,
  getLimitFontSize } from '../common/utils';
import Constants from '../common/constant';
import fs from '@ohos.file.fs';
import configPolicy from '@ohos.configPolicy';
import { EnableBgTaskAuthDialog } from '../ServiceExtAbility/NotificationServiceExtAbility';
import { Callback} from '@ohos.base';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import { MeasureOptions } from '@ohos.measure';
import { MeasureUtils } from '@ohos.arkui.UIContext';
import common from '@ohos.app.ability.common';

const TAG = 'BgTaskDialog_Service ';
const mainWindowId = 'mainWindow';
const permission: Record<string, Resource> = {
  'label': $r('app.string.text_notification_allow_background_keepalive'),
  'reason': $r('app.string.ohos_bgsubmode_prompt_media_process'),
};
const bottomPopoverTypes = ['default', 'phone'];

let storage = LocalStorage.getShared();

@Extend(Button) function customizeButton() {
  .backgroundColor(Color.Transparent)
  .fontColor($r('app.color.button_text'))
  .fontSize(
    getLimitFontSize(
      Constants.TEXT_MIDDLE_FONT_SIZE,
      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX))
  )
  .fontWeight(FontWeight.Medium)
  .height(Constants.BUTTON_HEIGHT)
  .flexGrow(Constants.FLEX_GROW)
  .width('50%')
}

@Extend(Button) function singleRowButton() {
  .backgroundColor(Color.Transparent)
  .fontColor($r('sys.color.font_emphasize'))
  .fontSize(
    getLimitFontSize(sourceToVp($r('sys.float.Body_L')),
      getFontSizeScale(getContext(this) as common.UIAbilityContext))
  )
  .fontWeight(FontWeight.Medium)
  .height(Constants.BUTTON_HEIGHT)
  .flexGrow(Constants.FLEX_GROW)
  .width('100%')
}

@Entry(storage)
@Component
struct NotificationDialogPage {
  @StorageLink('isUpdate') isUpdate: number = 0;

  privacyDialogController: CustomDialogController = new CustomDialogController({
    builder: PermissionDialog({ isUpdate: $isUpdate }),
    autoCancel: false,
    alignment: DialogAlignment.Center,
    customStyle: true,
    maskColor: $r('app.color.mask_thin'),
    onWillDismiss: (dismissDialogAction: DismissDialogAction) => {
      console.info(TAG, `dialog onWillDismiss reason= : ${JSON.stringify(dismissDialogAction.reason)}`);
    }
  });

  build() {}

  aboutToAppear() {
    this.privacyDialogController.open();
  }

  onPageShow() {
  }
}

@CustomDialog
struct PermissionDialog {
  @State appName: string = '';
  @State naviHeight: number = 0;
  @State isBottomPopover: boolean = true;
  @StorageLink('clicked') clicked: boolean = false;
  @Link @Watch('updateOnPageShow') isUpdate: number;
  dialog?: EnableBgTaskAuthDialog;
  session?: UIExtensionContentSession;
  controller?: CustomDialogController;
  @State titleContainerWidth: string | number = 'auto';
  @State maxHeight: string | number = '85%';
  @State taskMode: number = 0;

  build() {
    Row(){
      Flex({ justifyContent: FlexAlign.Center, alignItems: this.isBottomPopover ? ItemAlign.End : ItemAlign.Center }) {
        Column() {
          Scroll() {
            Column() {
              Row() {
                Flex({ justifyContent: FlexAlign.Center }) {
                  Text($r('app.string.text_notification_allow_background_keepalive', this.appName))
                  .fontSize($r('sys.float.ohos_id_text_size_headline8'))
                  .fontColor($r('sys.color.font_primary'))
                  .fontWeight(FontWeight.Bold)
                  .minFontSize(
                    getLimitFontSize(Constants.TITLE_MIN_FONT_SIZE,
                      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX))
                  )
                  .maxFontSize(
                    getLimitFontSize(sourceToVp($r('sys.float.ohos_id_text_size_headline8')),
                      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX))
                  )
                  .heightAdaptivePolicy(TextHeightAdaptivePolicy.MAX_LINES_FIRST)
                  .maxLines(2)
                  .textOverflow({overflow: TextOverflow.Ellipsis})
                  .width(this.titleContainerWidth)
                  .textAlign(TextAlign.Center)
                }
                .margin({
                  top: Constants.DIALOG_REQ_MARGIN_TOP,
                  bottom:Constants.DIALOG_REQ_MARGIN_BUTTOM,
                  left: Constants.DIALOG_REQ_MARGIN_LEFT,
                  right: Constants.DIALOG_REQ_MARGIN_RIGHT
                })
                .onSizeChange((oldValue: SizeOptions, newValue: SizeOptions) => {
                  let containerWidth = newValue.width as number;
                  let options: MeasureOptions = {
                    textContent: $r('app.string.text_notification_allow_background_keepalive', this.appName),
                    fontSize: getLimitFontSize(sourceToVp($r('sys.float.ohos_id_text_size_headline8')),
                      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX)),
                    fontWeight: FontWeight.Bold,
                  };
                  this.titleContainerWidth = calContainerWidth(containerWidth, options,
                    Constants.CROSS_LINE_RATIO, this.getUIContext().getMeasureUtils());
                  console.info(TAG, `onSizeChange titleContainerWidth: ${this.titleContainerWidth}`);
                })
              }
              Row() {
                Flex({ justifyContent: FlexAlign.Center }) {
                  Text() {
                    if (this.taskMode == Constants.SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION) {
                      Span($r('app.string.dialog_title_media_process'))
                    } else if (this.taskMode == Constants.SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION) {
                      Span($r('app.string.dialog_title_video_broadcast'))
                    } else if (this.taskMode == Constants.SUBMODE_WORK_OUT_NORMAL_NOTIFICATION) {
                      Span($r('app.string.dialog_title_work_out'))
                    }
                  }
                  .fontSize($r('sys.float.Subtitle_S'))
                  .fontWeight(FontWeight.Medium)
                  .fontColor($r('sys.color.font_primary'))
                  .lineHeight(Constants.DIALOG_DESP_LINE_HEIGHT)
                }
                .margin({
                  top: Constants.DIALOG_DESP_MARGIN_TOP,
                  left: Constants.DIALOG_DESP_MARGIN_LEFT,
                  right: Constants.DIALOG_DESP_MARGIN_RIGHT,
                  bottom: Constants.DIALOG_DESP_MARGIN_BOTTOM
                })
              }
              Row() {
                Flex({ justifyContent: FlexAlign.Center }) {
                  Button($r('app.string.btn_allow_time'))
                    .onClick(async (): Promise<void> => {
                      await this.enableNotification(Constants.AUTH_GRANTED_ONCE);
                    })
                    .singleRowButton()
                    .margin({ 
                      left: Constants.MARGIN_ZERO, 
                      right: Constants.MARGIN_ZERO
                    })
                }
                .margin({ 
                  left: Constants.BUTTON_MARGIN_LEFT,
                  right: Constants.BUTTON_MARGIN_RIGHT,
                  bottom: Constants.BUTTON_MARGIN_ROW
                })
              }
              Row() {
                Flex({ justifyContent: FlexAlign.Center }) {
                  Button($r('app.string.btn_allow_allowed'))
                    .onClick(async (): Promise<void> => {
                      await this.enableNotification(Constants.AUTH_GRANTED_ALWAYS);
                    })
                    .singleRowButton()
                    .margin({ 
                      left: Constants.MARGIN_ZERO, 
                      right: Constants.MARGIN_ZERO
                    })
                }
                .margin({ 
                  left: Constants.BUTTON_MARGIN_LEFT,
                  right: Constants.BUTTON_MARGIN_RIGHT,
                  bottom: Constants.BUTTON_MARGIN_ROW
                })
              }
              Row() {
                Flex({ justifyContent: FlexAlign.Center }) {
                  Button($r('app.string.btn_not_allowed'))
                  .onClick(async (): Promise<void> => {
                    await this.enableNotification(Constants.AUTH_DENIED);
                  })
                  .singleRowButton()
                  .margin({
                    left: Constants.MARGIN_ZERO,
                    right: Constants.MARGIN_ZERO,
                    bottom: Constants.BUTTON_MARGIN_TOP
                  })
                }
                .margin({ 
                  left: Constants.BUTTON_MARGIN_LEFT, 
                  right: Constants.BUTTON_MARGIN_RIGHT 
                })
              }

            }
          }
        }
        .borderRadius(Constants.DIALOG_PRIVACY_BORDER_RADIUS)
        .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
        .width(Constants.FULL_WIDTH)
        .clip(true)
        .id(mainWindowId)
        .onAreaChange((oldArea: Area, newArea: Area) => {
          this.updateButtomVisibility(newArea.width as number);
        })
      }
    }
    .margin({
      left: this.isBottomPopover ? Constants.DIALOG_MARGIN_VERTICAL : Constants.DIALOG_MARGIN,
      right: this.isBottomPopover ? Constants.DIALOG_MARGIN_VERTICAL : Constants.DIALOG_MARGIN,
      bottom: this.isBottomPopover ? this.naviHeight : 0
    })
    .constraintSize({ 
      maxHeight: this.maxHeight,
      maxWidth: Constants.MAX_DIALOG_WIDTH
    })
  }

  async updateApplicationName(bundleName: string): Promise<void> {
    console.info(TAG, `updateApplicationName bundleName: ${bundleName}`);
    try {
      let bundleFlags = bundleResourceManager.ResourceFlag.GET_RESOURCE_INFO_ALL;
      let resourceInfo = bundleResourceManager.getBundleResourceInfo(bundleName, bundleFlags);
      console.info(TAG, `applicationName name : ${JSON.stringify(resourceInfo.label)}`);
      let appName = resourceInfo.label;
      this.appName = titleTrim(appName);
      console.info(TAG, `hap label: ${this.appName}`);
    } catch (err) {
      console.error(TAG, `applicationName error : ${err?.code}`);
    }
  }

  updateAvoidWindow(): void {
    let type = window.AvoidAreaType.TYPE_SYSTEM;
    try {
      this.dialog?.extensionWindow.on('avoidAreaChange', (data): void => {
        if (data.type == window.AvoidAreaType.TYPE_SYSTEM) {
          console.info(TAG, `avoidAreaChange: ${JSON.stringify(data)}`);
          this.naviHeight = data.area.bottomRect.height;
        }
      });
      let avoidArea = this.dialog?.extensionWindow.getWindowAvoidArea(type);
      if (avoidArea != undefined) {
        console.info(TAG, `avoidArea: ${JSON.stringify(avoidArea)}`);
        this.naviHeight = avoidArea.bottomRect.height;
      }
    } catch (err) {
      console.error(TAG, `Failed to obtain the area. Cause: ${err?.code}`);
    }
  }

  updateSubWindowSize(): void {
    try {
      this.dialog?.extensionWindow.on('windowSizeChange', (data):void => {
        let windowRect = this.dialog?.extensionWindow.properties?.uiExtensionHostWindowProxyRect;
        console.info(TAG, `windowSizeChange event, size = ${windowRect?.left}-${windowRect?.top}-${windowRect?.width}-${windowRect?.height}`);
        if (!this.dialog?.initSubWindowSize && windowRect.width > 0 && windowRect.height > 0) {
          console.info(TAG, `windowSizeChange first time update`);
          this.dialog?.subWindow?.moveWindowToGlobal(windowRect?.left, windowRect?.top);
          this.dialog?.subWindow?.resize(windowRect?.width, windowRect?.height);
          this.dialog.initSubWindowSize = true;
        }
        this.updatemaxHeight();
      });
    } catch (err) {
      console.error(TAG, `updateSubWindowSize error. Cause: ${err?.code}`);
    }
  }

  updatemaxHeight(): void {
    try {
      let windowRect = this.dialog?.extensionWindow.properties?.uiExtensionHostWindowProxyRect;
      let height = windowRect?.height;
      let navigationArea = this.dialog?.extensionWindow.getWindowAvoidArea(
          window.AvoidAreaType.TYPE_NAVIGATION_INDICATOR);
      let navigationHeight = navigationArea?.bottomRect?.height;
      let systemArea = this.dialog?.extensionWindow.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM);
      let statusBarHeight = systemArea?.topRect?.height;
      console.info(TAG, `windowHeight ${windowRect?.height} 
          navigationHeight ${navigationHeight} statusBarHeight ${statusBarHeight} `);
      if (height > 0 && (height - navigationHeight - statusBarHeight) > 0 ) {
        this.maxHeight = px2vp((height - navigationHeight - statusBarHeight) * 0.9);
      }
    } catch (err) {
      console.error(TAG, `updatemaxHeight error. Cause: ${err?.code}`);
    }
  }

  updateButtomVisibility(dialogWidth: number): void {
    console.info(TAG, `updateButtomVisibility`);
  }

  async updateIsBottomPopover(): Promise<void> {
    let dis = display.getDefaultDisplaySync();
    let isVertical = dis.width <= dis.height;
    try {
      if (display.isFoldable()) {
        let foldStatus = display.getFoldStatus();
        if (foldStatus == display.FoldStatus.FOLD_STATUS_EXPANDED ||
          foldStatus == display.FoldStatus.FOLD_STATUS_HALF_FOLDED) {
            this.isBottomPopover = false;
            return;
        }
      }
    } catch (err) {
      console.error(TAG, 'Failed to get the device foldable status. Code: ${err?.code}');
    }

    // read ccm configs
    let isBottomPopoverTemp = false;
    this.isBottomPopover = isBottomPopoverTemp && isVertical;
  }

  async updateStatus(): Promise<void> {
    let bundleNameObj = this.dialog?.want.parameters?.bundleName;
    let bundleName = bundleNameObj ? bundleNameObj.toString() : '';
    this.taskMode = this.dialog?.want.parameters?.taskMode as number;
    console.info(TAG, `updateStatus taskMode: ${this.taskMode}`);
    await this.updateApplicationName(bundleName);
    await this.updateIsBottomPopover();
    try {
      let componentInfo = this.getUIContext().getComponentUtils().getRectangleById(mainWindowId);
      if (componentInfo?.size?.width > 0 ) {
        console.log(TAG, 'updateStatus-updateButtomVisibility');
        this.updateButtomVisibility(px2vp(componentInfo?.size?.width));
      }
    } catch (err) {
      console.error(TAG, `getRectangleById error : ${err?.code}`);
    } 
  }

  async updateOnPageShow(): Promise<void> {
    if (this.isUpdate > 0) {
      await this.updateStatus();
    }
  }

  async aboutToAppear(): Promise<void> {
    this.dialog = storage.get('dialog') as EnableBgTaskAuthDialog;
    this.session = storage.get('session') as UIExtensionContentSession;
    this.updateAvoidWindow();
    this.updateSubWindowSize();
    this.updatemaxHeight();
    try {
      await this.updateStatus();
    } catch (err) {
      console.error(TAG, `aboutToAppear error : ${err?.code}`);
      await this.dialog?.destroyException();
      await this.session?.terminateSelf();
    }
  }

  async registerFoldableCallback(): Promise<void> {
    let callback: Callback<display.FoldDisplayMode> = async (data: display.FoldDisplayMode) => {
      try {
        let win = this.dialog.window;
        let dis = display.getDefaultDisplaySync();
        await win.moveWindowTo(0, 0);
        await win.resize(dis.width, dis.height);
        await this.updateStatus();
      } catch (err) {
        console.error(TAG, 'Failed to touch callback. Code: ${err?.code}');
      }
    };
    try {
      display.on('foldDisplayModeChange', callback);
    } catch (err) {
      console.error(TAG, 'Failed to register callback. Code: ${err?.code}');
    }
  }

  async aboutToDisappear(): Promise<void> {
    console.info(TAG, `aboutToDisappear`);
    this.session?.terminateSelf();
  }

  async enableNotification(authType: number): Promise<void> {
    console.info(TAG, `NotificationDialog authType: ${authType}`);
    try {
      await this.dialog?.publishButtonClickedEvent(authType);
      this.clicked = true;
    } catch (err) {
      console.error(TAG, `NotificationDialog enable error, code is ${err?.code}`);
      await this.dialog?.destroyException();
    } finally {
      await this.dialog?.subWindow?.destroyWindow();
      this.session?.terminateSelf();
    }
  }
}

interface NotificationConfig {
  notificationAuthorizationWindow: NotificationAuthorizationWindow;
}

interface NotificationAuthorizationWindow {
  isBottomPopover: boolean;
}