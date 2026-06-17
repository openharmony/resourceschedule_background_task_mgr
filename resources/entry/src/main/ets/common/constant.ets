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

export default class Constants {
  public static DIALOG_MARGIN = 16;
  public static DIALOG_MARGIN_VERTICAL = 12;

  // 100% width,height
  public static FULL_WIDTH = '100%';
  public static FULL_HEIGHT = '100%';

  // public property style
  public static FLEX_GROW = 1;
  public static TEXT_MIDDLE_FONT_SIZE = 16;
  public static MAXIMUM_HEADER_LENGTH = 1000;

  // icon of dialog
  public static DIALOG_ICON_WIDTH = 32;
  public static DIALOG_ICON_HEIGHT = 32;
  public static DIALOG_ICON_MARGIN_TOP = 24;

  // request text of dialog
  public static DIALOG_REQ_MARGIN_TOP = 16;
  public static DIALOG_REQ_MARGIN_BUTTOM = 16;
  public static DIALOG_REQ_MARGIN_LEFT = 24;
  public static DIALOG_REQ_MARGIN_RIGHT = 24;

  // description text of dialog
  public static DIALOG_DESP_MARGIN_TOP = 0;
  public static DIALOG_DESP_MARGIN_LEFT = 24;
  public static DIALOG_DESP_MARGIN_RIGHT = 24;
  public static DIALOG_DESP_MARGIN_BOTTOM = 8;
  public static DIALOG_DESP_LINE_HEIGHT = 21;

  public static BUTTON_MARGIN_TOP = 8;
  public static BUTTON_MARGIN_LEFT = 16;
  public static BUTTON_MARGIN_RIGHT = 16;
  public static BUTTON_HEIGHT = 40;

  public static DIALOG_PRIVACY_BORDER_RADIUS = 32;
  public static MARGIN_ZERO = 0;
  public static BUTTON_MARGIN_ROW = 4;

  public static TITLE_MIN_FONT_SIZE = 16;
  public static CROSS_LINE_RATIO = 0.2;
  public static CROSS_LINE_RESIZE_PARAM = 1.25;

  public static FONT_SCALE_DEFAULT = 1;
  public static FONT_SCALE_MAX = 1.8;
  public static FONT_SCALE_MAX_PHONE = 2;
  public static MAX_DIALOG_WIDTH = 400;

  //pc
  public static PC_TITLE_MARGIN_TOP = 16;
  public static PC_TITLE_MARGIN_SIDE = 24;
  public static PC_CONTENT_MARGIN_TOP = 16;
  public static PC_CONTENT_MARGIN_SIDE = 24;
  public static PC_OPERATE_MARGIN_BUTTOM = 16;
  public static PC_DIALOG_WIDTH = 400;
  public static PC_BUTTON_HEIGHT = 40;
  public static PC_RDIUS_12 = 12;
  public static PC_RDIUS_8 = 8;
  public static PC_DOCK_HEIGHT = 120;
  public static PC_BUTTON_MARGIN_ROW = 8;
  public static PC_OUTLINE_COLOR = '#20FFFFFF'; 

  public static TAG = 'BgTaskDialog_Service ';

  // continuous task mode
  public static SUBMODE_MEDIA_PROCESS_NORMAL_NOTIFICATION = 9;
  public static SUBMODE_VIDEO_BROADCAST_NORMAL_NOTIFICATION = 10;
  public static SUBMODE_WORK_OUT_NORMAL_NOTIFICATION = 11;

  // authType
  public static AUTH_DENIED = 2;
  public static AUTH_GRANTED_ONCE = 3;
  public static AUTH_GRANTED_ALWAYS = 4;
}