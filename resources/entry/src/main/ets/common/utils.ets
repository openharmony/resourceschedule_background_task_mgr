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

import Constants from './constant';
import { MeasureOptions } from '@ohos.measure';
import { MeasureUtils } from '@ohos.arkui.UIContext';
import common from '@ohos.app.ability.common';

/**
 * Omit display when application name is too long
 * @param {String} Application name
 */
export function titleTrim(title: string): string {
  let length = title.length;
  if (length > Constants.MAXIMUM_HEADER_LENGTH) {
    let str = '';
    str = title.substring(0, Constants.MAXIMUM_HEADER_LENGTH) + '...';
    return str;
  } else {
    return title;
  }
}

export function calOverLine(containerWidth: number, options: MeasureOptions,
  measureUtils: MeasureUtils) : boolean {
    let crossLineFlag: boolean = false;
    try {
      let textWidth = px2vp(measureUtils.measureText(options));
      crossLineFlag = (textWidth > containerWidth);
    } catch (err) {
      console.error(Constants.TAG, `cal over line failed, code=${err?.code0}, msg=${err?.message}.`);
    }
    return crossLineFlag;
  }

export function calContainerWidth(
  containerWidth: number,
  options: MeasureOptions,
  crossLineRatio: number,
  measureUtils: MeasureUtils) : string | number {
    let targetContainerWidth: string | number = 'auto';
    try {
      let textWidth = px2vp(measureUtils.measureText(options));
      let ratio = (textWidth % containerWidth) / containerWidth;
      let crossLineFlag: boolean = (textWidth > containerWidth) && (textWidth < 2 * containerWidth);
      if (crossLineFlag && ratio < crossLineRatio) {
        targetContainerWidth = containerWidth / Constants.CROSS_LINE_RESIZE_PARAM;
        console.info(Constants.TAG, 'afford condition, cal new width.');
      }
    } catch (err) {
      console.error(Constants.TAG, `refresh width failed, code=${err?.code0}, msg=${err?.message}.`);
    }
    return targetContainerWidth;
  }

export function getFontSizeScale(context: common.UIAbilityContext, maxScale: number = 3.2): number {
  let fontSizeScale = context.config?.fontSizeScale;
  console.log(Constants.TAG, `fontSizeScale = ${fontSizeScale}`);
  if (fontSizeScale === undefined) {
    return Constants.FONT_SCALE_DEFAULT;
  }
  return fontSizeScale > maxScale ? maxScale : fontSizeScale;
}

export function sourceToVp(source: Resource): number {
  return px2vp(getContext().resourceManager.getNumber(source.id));
}

export function getLimitFontSize(fontSizeScale: number, sourceFontSize: number): string {
  return `${sourceFontSize * fontSizeScale}vp`;
}

export function shouldMoveToDisplayCenter(comW: number, comH: number,
  windowPoxy: uiExtension.WindowProxy,
  win: window.Window, dis: display.Display): boolean {
  try {
    if (windowPoxy === undefined) {
      console.error(Constants.TAG, `windowPoxy is null`);
      return false;
    }
    let windowRect = windowPoxy.properties?.uiExtensionHostWindowProxyRect;
    if (windowRect === undefined) {
      console.error(Constants.TAG, `windowRect is null`);
      return false;
    }
    if (windowRect.width <= 0 || windowRect.height <= 0) {
      console.error(Constants.TAG, `windowRect info error`);
      return false;
    }
    if (win === undefined) {
      console.error(Constants.TAG, `window is null`);
      return false;
    }
    if (dis === undefined) {
      console.error(Constants.TAG, `display is null`);
      return false;
    }
    
    console.info(Constants.TAG, `display W=${dis.width}, H=${dis.height} avaliableH=${dis.availableHeight}`);
    console.info(Constants.TAG, `compoment W=${comW}, H=${comH}`);
    console.info(Constants.TAG, `windowRect L=${windowRect.left} T=${windowRect.top} W=${windowRect.width} H=${windowRect.height}`);

    let leftMargin = (windowRect.width - comW) /2;
    let topMargin = (windowRect.height - comH) /2;
    let leftTopXPos = windowRect.left + leftMargin;
    let leftTopYPos = windowRect.top + topMargin;
    let rightBottomXPos = leftTopXPos + comW;
    let rightBottomYPos = leftTopYPos + comH;
    console.info(Constants.TAG, `comPos info LTX=${leftTopXPos} LTY=${leftTopYPos} RBX=${rightBottomXPos} RBY=${rightBottomYPos}`);

    if (leftTopXPos < 0 || rightBottomXPos > dis.width || leftTopYPos < 0) {
      return true;
    }

    let displayId = win.getWindowProperties().displayId;
    let foldStatus = display.getFoldStatus();
    let disPlayValidHeight = 0;
    let chargeCrossAxis = false;
    if (foldStatus === display.FoldStatus.FOLD_STATUS_EXPANDED) {
      disPlayValidHeight = dis.height - Constants.PC_DOCK_HEIGHT; 
    } else {
      disPlayValidHeight = (dis.availableHeight !== undefined && dis.availableHeight <= dis.height) ?
        dis.availableHeight : dis.height;
      if (displayId === DEFAULT_DISPLAY_ID) {
        chargeCrossAxis = true;
      }
    }

    if (chargeCrossAxis) {
      if (disPlayValidHeight > leftTopYPos && disPlayValidHeight < rightBottomYPos) {
          return true;
        }
    } else {
      if (rightBottomYPos > disPlayValidHeight) {
          return true;
      }
    }
  } catch (err) {
    console.error(Constants.TAG, `shouldMoveToDisplayCenter failed, code=${err?.code}, msg=${err?.message}.`);
  }
  return false;
}

export async function sleep(ms: number): Promise<void> {
  return new Promise(resolve => setTimeout(resolve, ms));
}

/**
   * 
   * @param windowRect windowRect
   * @param display display
   * @returns true need removeWindow
   */
  export function shouldMoveWindow(windowRect: window.Rect, display: display.Display): boolean {
    try {
      if (windowRect === undefined) {
        console.error(Constants.TAG, `windowRect is null`);
        return false;
      }
      console.info(Constants.TAG, `displayW=${display.width}, displayH=${display.height}.` );

      if (windowRect.width >= display.width && windowRect.height >= display.height) {
        console.info(Constants.TAG, `windowRect >= display, no need move.`);
        return false;
      }
      let windowCenterX = windowRect.left + windowRect.width / 2;
      let windowCenterY = windowRect.top + windowRect.height / 2;
      let minX = windowRect.width / 2;
      let maxX = display.width - windowRect.width / 2;
      let minY = windowRect.height / 2;
      let maxY = display.height - windowRect.height / 2 - Constants.PC_DOCK_HEIGHT;

      console.info(Constants.TAG, `windowCenterX=${windowCenterX}, windowCenterY=${windowCenterY}`);
      console.info(Constants.TAG, `x range=[${minX}, ${maxX}], y range=[${minY}, ${maxY}]`);
      let isXOk: boolean = windowCenterX >= minX && windowCenterX <= maxX;
      let isYOk: boolean = windowCenterY >= minY && windowCenterY <= maxY;
      let shouldMove = !(isXOk && isYOk);
      console.info(Constants.TAG, `shouldMoveWindow, shouldMove=${shouldMove}`);
      return shouldMove;
    } catch (err) {
      console.error(Constants.TAG, `check shouldMoveWindow failed, code=${err?.code}, msg=${err?.message}.`);
      return false;
    }
  }
