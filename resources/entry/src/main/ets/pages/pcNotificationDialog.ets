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
  getLimitFontSize,
  shouldMoveToDisplayCenter,
  sleep
} from '../common/utils';
import Constants from '../common/constant';
import fs from '@ohos.file.fs';
import configPolicy from '@ohos.configPolicy';
import { EnableBgTaskAuthDialog } from '../ServiceExtAbility/NotificationServiceExtAbility';
import { Callback} from '@ohos.base';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import { MeasureOptions } from '@ohos.measure';
import { MeasureUtils } from '@ohos.arkui.UIContext';
import common from '@ohos.app.ability.common';
import uiextension from '@ohos.arkui.uiExtension';
import DisplayUtils from '../common/displayUtils';

const TAG = 'BgTaskDialog_Service ';
const MASK_COLOR = '#33000000';
const TRANSPARANT_COLOR = '#00000000';
const EMPTY_PAGE_PATH = 'emptyPage';

const permission: Record<string, Resource> = {
  'label': $r('app.string.text_notification_allow_background_keepalive'),
  'reason': $r('app.string.ohos_bgsubmode_prompt_media_process'),
};

let storage = LocalStorage.getShared();

@Extend(Button) function customizeButton() {
  .type(ButtonType.Normal)
  .backgroundColor($r('sys.color.comp_background_tertiary'))
  .fontSize(
    getLimitFontSize(
      sourceToVp($r('sys.float.Body_L')),
      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX))
  )
  .fontWeight(FontWeight.Medium)
  .height(Constants.PC_BUTTON_HEIGHT)
  .width('50%')
  .borderRadius(Constants.PC_RDIUS_8)
  .flexGrow(Constants.FLEX_GROW)
}

@Extend(Button) function singleRowButton() {
  .type(ButtonType.Normal)
  .backgroundColor($r('sys.color.comp_background_tertiary'))
  .fontColor($r('app.color.button_text'))
  .fontSize(
    getLimitFontSize(
      Constants.TEXT_MIDDLE_FONT_SIZE,
      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX))
  )
  .fontWeight(FontWeight.Medium)
  .height(Constants.BUTTON_HEIGHT)
  .borderRadius(Constants.PC_RDIUS_8)
  .flexGrow(Constants.FLEX_GROW)
  .width('100%')
}

@Extend(Text) function contentText() {
  .fontSize(
    getLimitFontSize(sourceToVp($r('sys.float.Subtitle_S')),
      getFontSizeScale(getContext(this) as common.UIAbilityContext))
  )
  .fontWeight(FontWeight.Medium)
  .fontColor($r('sys.color.font_primary'))
}

@Entry(storage)
@Component
struct NotificationDialogPage {
  @StorageLink('isUpdate') isUpdate: number = 0;
  @StorageLink('uecRectChangeNum') uecRectChangeNum: number = 0;

  privacyDialogController: CustomDialogController = new CustomDialogController({
    builder: PermissionDialog({ 
      isUpdate : $isUpdate,
      uecRectChangeNum : $uecRectChangeNum
    }),
    autoCancel: false,
    alignment: DialogAlignment.Center,
    customStyle: true,
    cornerRadius: Constants.PC_RDIUS_12,
    maskColor: $r('sys.color.ohos_id_color_mask_thin'),
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
  @State contentComW: number = 0;
  @State contentComH: number = 0;
  @State showInDisplayCenter: boolean = false;
  @State followParentMode: boolean = true;
  @State waterFallMode: boolean = false;
  @State contentSizeChangeUpdated: boolean = false;
  @State firstUpdateCompoment:boolean = true;
  @State isBottomPopover: boolean = false;
  @StorageLink('clicked') clicked: boolean = false;
  @Link @Watch('updateOnPageShow') isUpdate: number;
  @Link @Watch('updateOnPageShow') uecRectChangeNum: number;
  dialog?: EnableBgTaskAuthDialog;
  session?: UIExtensionContentSession;
  controller?: CustomDialogController;
  @State titleContainerWidth: string | number = 'auto';
  @State taskMode: number = 0;

  build() {
    Row() {
      Flex({ justifyContent: FlexAlign.Center, alignItems: this.isBottomPopover ? ItemAlign.End : ItemAlign.Center }) {
        Column() {
          Scroll() {
            Column() {
              Row() {
                Flex({ justifyContent: FlexAlign.Center }) {
                  Text($r('app.string.text_notification_allow_background_keepalive', this.appName))
                  .fontSize($r('sys.float.Title_S'))
                  .fontColor($r('sys.color.font_primary'))
                  .fontWeight(FontWeight.Bold)
                  .minFontSize(
                    getLimitFontSize(Constants.TITLE_MIN_FONT_SIZE,
                      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX))
                  )
                  .maxFontSize(
                    getLimitFontSize(sourceToVp($r('sys.float.Title_S')),
                      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX))
                  )
                  .heightAdaptivePolicy(TextHeightAdaptivePolicy.MAX_LINES_FIRST)
                  .maxLines(2)
                  .textOverflow({overflow: TextOverflow.Ellipsis})
                  .width(this.titleContainerWidth)
                  .textAlign(TextAlign.Center)
                }
                .margin({
                  left: Constants.PC_TITLE_MARGIN_SIDE,
                  right: Constants.PC_TITLE_MARGIN_SIDE,
                  top: Constants.PC_TITLE_MARGIN_TOP,
                })
                .onSizeChange((oldValue: SizeOptions, newValue: SizeOptions) => {
                    let containerWidth = newValue.width as number;
                    let options: MeasureOptions = {
                      textContent: $r('app.string.text_notification_allow_background_keepalive', this.appName),
                      fontSize: getLimitFontSize(sourceToVp($r('sys.float.Title_S')),
                      getFontSizeScale(getContext(this) as common.UIAbilityContext, Constants.FONT_SCALE_MAX)),
                      fontWeight: FontWeight.Bold,
                    };
                    this.titleContainerWidth = calContainerWidth(containerWidth, options,
                      Constants.CROSS_LINE_RATIO, this.getUIContext().getMeasureUtils());
                    console.info(TAG, `onSizeChange titleContainerWidth: ${this.titleContainerWidth}`);
                  })
              }
              Row() {
                Flex({ justifyContent: FlexAlign.Center, direction: FlexDirection.Column }) {
                  Text() {
                    if (this.taskMode == Constants.SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION) {
                      Span($r('app.string.dialog_title_media_process'))
                    } else if (this.taskMode == Constants.SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION) {
                      Span($r('app.string.dialog_title_video_broadcast'))
                    } else if (this.taskMode == Constants.SUBMODE_WORK_OUT_NORMAL_NOTIFICATION) {
                      Span($r('app.string.dialog_title_work_out'))
                    }
                  }
                  .contentText()
                }
                .margin({
                  left: Constants.PC_CONTENT_MARGIN_SIDE,
                  right: Constants.PC_CONTENT_MARGIN_SIDE,
                  top: Constants.PC_CONTENT_MARGIN_TOP,
                  bottom: Constants.PC_TITLE_MARGIN_TOP
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
                  bottom: Constants.PC_BUTTON_MARGIN_ROW
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
                  bottom: Constants.PC_BUTTON_MARGIN_ROW
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
                    bottom: Constants.PC_OPERATE_MARGIN_BUTTOM
                  })
                }
                .margin({ 
                  left: Constants.BUTTON_MARGIN_LEFT, 
                  right: Constants.BUTTON_MARGIN_RIGHT 
                })
              }
            }
            .onSizeChange((oldValue: SizeOptions, newValue: SizeOptions) => {
              this.contentComW = vp2px(newValue.width as number);
              this.contentComH = vp2px(newValue.height as number);
              console.info(TAG, `content onSizeChange size ${this.contentComW} ${this.contentComH}`);
              if (!this.contentSizeChangeUpdated) {
                this.updateContentCompoment();
                this.contentSizeChangeUpdated = true;
              }
            })
          }
        }
        .borderRadius(Constants.PC_RDIUS_12)
        .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
        .width(Constants.PC_DIALOG_WIDTH)
        .constraintSize({minWidth:Constants.PC_DIALOG_WIDTH})
        .outline({
            width: 1,
            color: Constants.PC_OUTLINE_COLOR,
            radius: Constants.PC_RDIUS_12
          })
        .clip(true)
      }
      .width(Constants.FULL_WIDTH)
      .height(Constants.FULL_HEIGHT)
    }
  }

  async aboutToAppear(): Promise<void> {
    this.dialog = storage.get('dialog') as EnableBgTaskAuthDialog;
    this.session = storage.get('session') as UIExtensionContentSession;
    this.updateAvoidWindow();
    this.registerFoldStatusChange();
    try {
      await this.updateStatus();
    } catch (err) {
      console.error(TAG, `aboutToAppear error : ${err?.code}`);
      await this.dialog?.destroyException();
      await this.session?.terminateSelf();
    }
  }

  async aboutToDisappear(): Promise<void> {
    console.info(TAG, `aboutToDisappear`);
    this.session?.terminateSelf();
  }

  async updateOnPageShow(): Promise<void> {
    if (this.isUpdate > 0) {
      await this.updateStatus();
      await this.updateContentCompoment();
      this.isUpdate = 0;
    }
    if (this.uecRectChangeNum > 0) {
      await this.updateContentCompoment();
      this.uecRectChangeNum = 0;
    }
  }

  async updateStatus(): Promise<void> {
    let bundleNameObj = this.dialog?.want.parameters?.bundleName;
    let bundleName = bundleNameObj ? bundleNameObj.toString() : '';
     this.taskMode = this.dialog?.want.parameters?.taskMode as number;
    await this.updateApplicationName(bundleName);
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
  
  async updateContentCompoment(): Promise<void> {
    console.info(TAG, `updateContentCompoment`);
    if (this.contentComW <= 0 || this.contentComH <= 0) {
      console.error(TAG, `contentCompoment info not init`);
      return;
    }
    if (!this.firstUpdateCompoment) {
      await sleep(150);
    } else {
      this.firstUpdateCompoment = false;
    }
    
    try {
      let queryWaterFallMode = await this.dialog?.subWindow.isMainWindowFullScreenAcrossDisplays();
      this.waterFallMode = this.waterFallMode || queryWaterFallMode;
      let display = DisplayUtils.getCurWindowDisplay(this.dialog?.subWindow);
      this.showInDisplayCenter = shouldMoveToDisplayCenter(this.contentComW, this.contentComH,
        this.dialog?.extensionWindow, this.dialog?.subWindow, display);
      console.info(TAG, `shouldMoveToCenter = ${this.showInDisplayCenter} waterFallMode = ${this.waterFallMode}`);

      if (this.showInDisplayCenter) {
        await this.moveToDisplayCenter();
      } else {
        if (this.waterFallMode) {
          await this.moveToParentWindowCenter();
        } else {
          await this.follwParentWindow();
        }
        
      }
    } catch (err) {
      console.error(TAG, `updateContentCompoment fail. code=${err?.code} msg=${err?.message}`);
    }
  }

  async moveToParentWindowCenter(): Promise<void> {
    try {
      if (this.followParentMode) {
        await this.dialog?.subWindow?.setFollowParentWindowLayoutEnabled(false);
        this.session?.loadContent(EMPTY_PAGE_PATH, storage);
        this.session?.setWindowBackgroundColor(MASK_COLOR);
        this.followParentMode = false;
      }
      let windowRect = this.dialog?.extensionWindow.properties?.uiExtensionHostWindowProxyRect;
      if (!windowRect || windowRect.width <= 0 || windowRect.height <= 0) {
        console.error(TAG, `unavaliable windowRect info`);
        return;
      }
      this.dialog?.subWindow?.resize(this.contentComW, this.contentComH);
      this.dialog?.subWindow?.moveWindowTo((windowRect.width - this.contentComW) / 2 + windowRect.left,
        (windowRect.height - this.contentComH) / 2 + windowRect.top);
    } catch (err) {
      console.error(TAG, `moveToParentWindowCenter fail. code=${err?.code} msg=${err?.message}`);
    }
  }

  async moveToDisplayCenter(): Promise<void> {
    try {
      if (this.followParentMode) {
        await this.dialog?.subWindow?.setFollowParentWindowLayoutEnabled(false);
        this.session?.loadContent(EMPTY_PAGE_PATH, storage);
        this.session?.setWindowBackgroundColor(MASK_COLOR);
        this.followParentMode = false;
      }
      let display = DisplayUtils.getCurWindowDisplay(this.dialog?.subWindow);
      this.dialog?.subWindow?.resize(this.contentComW, this.contentComH);
      this.dialog?.subWindow?.moveWindowTo((display.width - this.contentComW) / 2,
         (display.height - this.contentComH) / 2);
    } catch (err) {
      console.error(TAG, `moveToDisplayCenter fail. code=${err?.code} msg=${err?.message}`);
    }
  }

  async follwParentWindow(): Promise<void> {
    try {
      if (this.followParentMode) {
        return;
      }
      await this.dialog?.subWindow?.setFollowParentWindowLayoutEnabled(true);
      this.session?.setWindowBackgroundColor(TRANSPARANT_COLOR);
      this.followParentMode = true;
    } catch (err) {
      console.error(TAG, `follwParentWindow fail. code=${err?.code} msg=${err?.message}`);
    }
  }

  async registerFoldStatusChange(): Promise<void> {
    let callback: Callback<display.FoldStatus> = async (data: display.FoldStatus) => {
      try {
        console.info(TAG, `foldStatusChange waterFallMode = ${this.waterFallMode}`);
        let dis = DisplayUtils.getCurWindowDisplay(this.dialog?.subWindow);
        let windowRect = this.dialog?.extensionWindow.properties?.uiExtensionHostWindowProxyRect;
        if (this.waterFallMode || data === display.FoldStatus.FOLD_STATUS_EXPANDED ||
          (windowRect && windowRect.height > dis.height)) {
          this.updateContentCompoment();
        }
      } catch (err) {
        console.error(TAG, `Failed to touch foldStatusChange callback. code=${err?.code} msg=${err?.message}`);
      }
    };
    try {
      display.on('foldStatusChange', callback);
    } catch (err) {
      console.error(TAG, `Failed to register foldStatusChange event. code=${err?.code} msg=${err?.message}`);
    }
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