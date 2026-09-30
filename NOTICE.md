# NOTICE — 使用している既存ソフトウェアと著作権表示

本リポジトリには、応募者が作成したコードのほかに、以下の既存ソフトウェアが含まれています。
各ソフトウェアの著作権はそれぞれの著作権者に帰属し、各ライセンスの条件に従って利用・再配布しています。
ソースファイル先頭の著作権表示・ライセンス表記は改変せずに残しています。

いずれも無償で公開されており、本コンテストの表彰式終了後も下記の入手先から入手できます。

| # | ソフトウェア | 著作権者 | ライセンス | 配置場所 | 入手先 |
|---|---|---|---|---|---|
| 1 | μT-Kernel 3.0 BSP2 | TRON フォーラム | T-License 2.2 | `mtk3bsp2_ra8p1_ek/mtk3_bsp2/` | https://github.com/tron-forum/mtk3_bsp2 , https://github.com/tron-forum/mtk3bsp2_samples |
| 2 | Renesas Flexible Software Package (FSP) v6.6.0（CMSIS を含む） | Renesas Electronics Corporation / Arm Limited | BSD-3-Clause / Apache-2.0 (CMSIS) | `*/ra/`, `*/ra_gen/`, `*/ra_cfg/`, `*/script/`, `tools/linker/`（e² studio が生成したリンカ設定） | https://github.com/renesas/fsp |
| 3 | EK-RA8P1 Vision AI 顔検出サンプル（`ek_ra8p1_vision_face_detection_yolo_fastest_FSP660`） | Renesas Electronics Corporation | BSD-3-Clause | `mtk3bsp2_ra8p1_ek/src/`（camera_layer, display_layer, external_memory, time_counter, *_thread_entry.c, common_util.* 等。μT-Kernel 対応のため改変） | https://github.com/renesas/ruhmi-framework-mcu |
| 4 | RUHMI Framework による変換済みモデルコード（YOLO-fastest） | EdgeCortix Inc. / Renesas Electronics Corporation | Apache-2.0 | `mtk3bsp2_ra8p1_ek/src/ai_application/ruhmi_conversion_results/` | https://github.com/renesas/ruhmi-framework-mcu |
| 5 | 推論後処理・画像処理ユーティリティ（Arm ML Embedded Evaluation Kit 由来） | Arm Limited | Apache-2.0 | `mtk3bsp2_ra8p1_ek/src/ai_application/common/`, `face_detection/` | https://github.com/renesas/ruhmi-framework-mcu |
| 6 | TensorFlow Lite for Microcontrollers C API ヘッダ（型定義のみ） | The TensorFlow Authors | Apache-2.0 | `mtk3bsp2_ra8p1_ek/src/ai_application/tflite_headers/` | https://github.com/tensorflow/tflite-micro |
| 7 | SEGGER RTT | SEGGER Microcontroller GmbH | SEGGER RTT ライセンス（BSD 系） | `mtk3bsp2_ra8p1_ek/src/SEGGER_RTT*.h` | https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/ |

## ライセンス本文

- T-License 2.2: https://www.tron.org/download/index.php?route=information/information&information_id=79
- BSD-3-Clause: https://opensource.org/license/bsd-3-clause
- Apache License 2.0: http://www.apache.org/licenses/LICENSE-2.0

## 応募者が作成した部分

`Application/` 以下（6 タスク・転倒判定）、`src/rtos_to_mtk.h`・`src/app_config.h`・`src/app_hooks.h`、各ソースへの μT-Kernel 対応の改変、`src/hal_warmstart.c` の LCD リセット処理、`tests/`・`tools/`・`docs/` 以下は応募者（田中 真）が作成したもので、[MIT License](LICENSE) で公開します。
既存ソフトウェアのファイルに加えた改変部分は、元のファイルのライセンスに従います。
