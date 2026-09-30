# プライバシー保護型・転倒検知エッジAI（μT-Kernel 3.0 × Ethos-U55 on EK-RA8P1）

**Privacy-First Fall Detection Edge AI — μT-Kernel 3.0 on Renesas EK-RA8P1 (Cortex-M85 + Arm Ethos-U55 NPU)**

TRONプログラミングコンテスト2026 RTOSアプリケーション部門 応募作品（エントリー番号 51802）

カメラ映像をクラウドへ送らず、マイコンの中だけで高齢者の転倒を検知・通知する見守りシステムです。
NPU（Ethos-U55）で画像認識を行い、μT-Kernel 3.0 のリアルタイム制御で「検知から通知まで」を確実に回します。
LCD には生の映像を映さず、状態と検出枠だけを表示して、見守られる人のプライバシーを守ります。

---

## ⚠️ 現在の開発状況（2026-09-30 更新）

開発途中で**貸与ボードの LCD が故障**し（工場出荷時デモでも表示されないことを確認済み）、事務局のご了承を得て**代替機の到着待ち**です。
6 タスク構成・転倒判定までの実装は完了し、**ビルドとロジックの単体テストは通っていますが、実機での動作確認は代替機の到着後**になります。

| 項目 | 状況 |
|---|---|
| μT-Kernel 3.0 (mtk3_bsp2) の起動、複数タスク・LED・シリアル出力 | ✅ 実機で動作確認済み |
| 公式 AI サンプル（FreeRTOS 版・YOLO-fastest 顔検出）の実機動作 | ✅ 実機で確認済み（カメラ約30fps、推論約20回/秒） |
| FreeRTOS → μT-Kernel 互換レイヤー（`rtos_to_mtk.h`）、FSP 設定・ピンの移植 | ✅ 実装済み（カメラ画像の受信まで実機で確認） |
| **6 タスク構成**（アラート／システム監視／カメラ／転倒判定／AI 推論／UI） | ✅ 実装済み・ビルド確認済み / ⏳ 実機確認待ち |
| **転倒判定ロジック**（`fall_judge.c`） | ✅ 実装済み・PC 上の単体テスト 13 項目合格 / ⏳ 実機でのしきい値調整待ち |
| **プライバシー保護 UI**（映像を出さず状態と枠だけを表示） | ✅ 実装済み・ビルド確認済み / ⏳ 実機確認待ち（LCD 故障のため） |
| 遅延計測（フレーム取得 → アラート LED）、システム監視（周期ハンドラ＋ハートビート） | ✅ 実装済み / ⏳ 実機での計測待ち |
| 検知率・誤報率の評価、デモ動画 | ⏳ 代替機到着後に実施 |

**ビルド確認の方法**: e² studio と同じ Arm GNU Toolchain 13.2.1・同じコンパイルオプションで、プロジェクトの全ソース（1086 ファイル）のコンパイルとリンクが警告なし（応募者作成部分）で通ることを確認しています（`tools/build_gcc.py`）。

### ブランチ

| ブランチ | 内容 |
|---|---|
| `Blinky_Test` | **応募版（既定ブランチ、応募フォームに登録した URL）**。6 タスク実装を含む最新版 |
| `develop` | 開発用（代替機での確認・調整はここで行い、確認後に `Blinky_Test` へ反映） |
| `submission-2026-09-30` | 2026-09-30 夕方の初回提出時点の版（6 タスク実装前、記録用） |

---

## システム構成

| 要素 | 内容 |
|---|---|
| ボード | Renesas **EK-RA8P1**（RA8P1: Cortex-M85 1GHz ＋ Cortex-M33、Arm Ethos-U55 NPU）※CM85 のみ使用 |
| OS | **μT-Kernel 3.0**（TRON フォーラム mtk3_bsp2、RA FSP 版） |
| ドライバ | Renesas FSP 6.6.0（r_vin, r_mipi_csi, r_mipi_phy, r_glcdc, r_drw, rm_ethosu, r_iic_master, r_gpt） |
| カメラ | キット同梱 OV5640（MIPI CSI-2）、640×480 取り込み |
| 表示 | キット同梱 7 インチ 1024×600 LCD（グラフィックス拡張ボード、パラレル RGB） |
| AI | YOLO-fastest（RUHMI Framework で Ethos-U55 向けに変換済み、入力 192×192 INT8、モデルは内蔵フラッシュに配置） |
| 通知 | ボード上の LED、LCD、シリアル出力 |

**キット同梱品だけで動作します。追加のハードウェア（外付け部品）は不要です。**

### μT-Kernel タスク構成（数値が小さいほど高優先）

| 優先度 | タスク | 役割 | 同期 |
|---|---|---|---|
| 5 | alert | 転倒確定で赤 LED 点灯 → シリアルに通報・遅延を表示 | イベントフラグ待ち |
| 8 | sysmon | 周期ハンドラで毎秒起床し、各タスクのハートビートを監視 | 周期ハンドラ → `tk_wup_tsk` |
| 10 | camera | フレーム取得・AI 入力の前処理（ルネサスサンプルを移植） | 割り込み → イベントフラグ |
| 11 | judge | 検出枠の時系列から転倒を判定（`fall_judge.c`） | メッセージバッファ受信 |
| 12 | ai | Ethos-U55 で推論し、最大の検出枠を judge へ送る（移植） | メッセージバッファ送信 |
| 14 | ui | 状態の色と抽象化した枠だけを描画（**カメラ映像は表示しない**） | 周期起床 |

```
 OV5640 ─MIPI─▶ VIN ─割込み─▶ [camera 10] ─フラグ─▶ [ai 12] ─メッセージバッファ─▶ [judge 11] ─フラグ─▶ [alert 5] ─▶ 赤LED
                                                      Ethos-U55                         │状態
                                                                                        ▼
                         周期ハンドラ ─▶ [sysmon 8]（ハートビート監視）          [ui 14] ─▶ LCD（状態のみ）
```

設計の詳細・転倒判定アルゴリズムは [docs/design.md](docs/design.md) を参照してください。

---

## リポジトリ構成

```
.
├── mtk3bsp2_ra8p1_ek/          ★応募プログラム本体（e² studio プロジェクト）
│   ├── Application/             ★自作アプリケーション
│   │   ├── app_main.c           usermain()：6 タスク・イベントフラグ・メッセージバッファ・ミューテックスの生成
│   │   ├── app_alert_task.c     アラートタスク
│   │   ├── app_sysmon_task.c    システム監視タスク（周期ハンドラ）
│   │   ├── app_judge_task.c     転倒判定タスク
│   │   ├── app_ui_task.c        プライバシー保護 UI タスク
│   │   ├── fall_judge.c/.h      転倒判定ロジック（OS 非依存・PC でテスト可能）
│   │   └── app_common.c/.h      共有オブジェクト・ログ・移植スレッドからのフック
│   ├── src/app_config.h         優先度・スタック・判定しきい値などの設定
│   ├── src/rtos_to_mtk.h        FreeRTOS API → μT-Kernel API 互換レイヤー（自作）
│   ├── src/camera_display_thread_entry.c   カメラ取得・前処理・表示（移植）
│   ├── src/ai_inference_thread_entry.c     NPU 推論（移植）
│   ├── src/hal_warmstart.c      LCD パネルのリセット解除処理を追加
│   ├── src/ai_application/      推論・後処理・変換済みモデル（ルネサス/Arm/EdgeCortix 提供）
│   ├── src/camera_layer/, src/display_layer/   カメラ・LCD ドライバ層（ルネサス提供を改変）
│   ├── mtk3_bsp2/               μT-Kernel 3.0 BSP2（TRON フォーラム）
│   └── configuration.xml        FSP 設定
├── tests/                       転倒判定ロジックの単体テスト（PC 上で `make`）
├── tools/build_gcc.py           e² studio なしでのビルド確認スクリプト
├── Blinky_Test/                 開発環境確認用の Lチカ（ベアメタル）
└── docs/
    ├── setup_guide.md           ★動作確認手順書（ビルド・書き込み・確認方法）
    ├── design.md                設計書（タスク構成・転倒判定）
    ├── porting_notes.md         FreeRTOS サンプル → μT-Kernel 移植ノウハウ集
    ├── presentation.pptx        紹介スライド（PowerPoint）
    └── presentation.pdf         紹介スライド（PDF・ブラウザで閲覧可）
```

---

## ビルドと実行（概要）

詳しい手順は **[docs/setup_guide.md](docs/setup_guide.md)** にあります。

1. **e² studio 2026-07 ＋ FSP 6.6.0**（FSP Platform Installer）をインストール（ツールチェーン: GCC ARM Embedded 13.2.1）
2. このリポジトリを clone し、e² studio で `mtk3bsp2_ra8p1_ek` を「既存プロジェクトのインポート」
3. `configuration.xml` を開き **Generate Project Content** → **Build**
4. EK-RA8P1 の **SW4 を 8 個すべて OFF**、カメラ・LCD 拡張ボードを取り付け、**J10（USB Debug）** を PC に接続
5. `mtk3bsp2_ra8p1_ek Debug_Flat` でデバッグ実行 → Resume
6. LCD の色と文字、シリアル出力（115200bps）で状態を確認

転倒判定ロジックだけなら、ボードなしで PC 上でテストできます: `cd tests && make`

---

## ライセンスと使用している既存ソフトウェア

本リポジトリは**オープンソースとして公開**しています。

- 応募者が作成した部分（`Application/` 以下、`src/rtos_to_mtk.h`・`src/app_config.h`・`src/app_hooks.h`、各ファイルの μT-Kernel 対応の改変、`tests/`・`tools/`・`docs/` 以下）: **MIT License**（[LICENSE](LICENSE)）
- 既存ソフトウェアはそれぞれのライセンスに従います。一覧と著作権表示は **[NOTICE.md](NOTICE.md)** を参照してください。

| ソフトウェア | 提供元 | ライセンス | 入手先 |
|---|---|---|---|
| μT-Kernel 3.0 BSP2 | TRON フォーラム | T-License 2.2 | https://github.com/tron-forum/mtk3_bsp2 |
| Flexible Software Package (FSP) 6.6.0 | Renesas Electronics | BSD-3-Clause | https://github.com/renesas/fsp |
| EK-RA8P1 Vision AI 顔検出サンプル（YOLO-fastest, FSP660） | Renesas Electronics | BSD-3-Clause | https://github.com/renesas/ruhmi-framework-mcu |
| RUHMI 変換済みモデルコード | EdgeCortix / Renesas | Apache-2.0 | 同上 |
| 推論後処理・画像処理（ml-embedded-evaluation-kit 由来） | Arm Limited | Apache-2.0 | 同上 |
| TensorFlow Lite Micro ヘッダ（型定義のみ） | The TensorFlow Authors | Apache-2.0 | https://github.com/tensorflow/tflite-micro |
| SEGGER RTT | SEGGER Microcontroller | SEGGER BSD-style | https://www.segger.com |

いずれも無償で公開されており、表彰式終了後も引き続き入手可能です。

---

## 今後の予定（代替機到着後）

1. 6 タスク構成・プライバシー UI の実機動作確認
2. 転倒判定しきい値（`src/app_config.h` の `APP_FJ_*`）の実機調整
3. 検知率・誤報率、「フレーム取得 → アラート LED」の遅延計測、デモ動画の撮影
4. 人物（person）検出モデルへの置き換え（`APP_FJ_USE_ASPECT=1` で検出枠の縦横比も判定に使用）

## 作者

田中 真 — GitHub: [@syouun](https://github.com/syouun)
