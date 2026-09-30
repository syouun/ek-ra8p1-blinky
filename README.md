# プライバシー保護型・転倒検知エッジAI（μT-Kernel 3.0 × Ethos-U55 on EK-RA8P1）

**Privacy-First Fall Detection Edge AI — μT-Kernel 3.0 on Renesas EK-RA8P1 (Cortex-M85 + Arm Ethos-U55 NPU)**

TRONプログラミングコンテスト2026 RTOSアプリケーション部門 応募作品（エントリー番号 51802）

カメラ映像をクラウドへ送らず、マイコンの中だけで高齢者の転倒を検知・通知する見守りシステムです。
NPU（Ethos-U55）で画像認識を行い、μT-Kernel 3.0 のリアルタイム制御で「検知から通知まで」を確実に回します。
LCD には生の映像を映さず、状態と検出枠だけを表示して、見守られる人のプライバシーを守ります。

---

## ⚠️ 現在の開発状況（2026-09-30 提出時点）

開発途中で**貸与ボードの LCD が故障**し（工場出荷時デモでも表示されないことを確認済み）、事務局のご了承を得て**代替機の到着待ち**の状態で提出しています。代替機での確認後、更新版を事務局へご連絡します。

| 項目 | 状況 |
|---|---|
| μT-Kernel 3.0 (mtk3_bsp2) の起動、複数タスク・LED・シリアル出力 | ✅ 実機で動作確認済み |
| 公式 AI サンプル（FreeRTOS 版・YOLO-fastest 顔検出）の実機動作 | ✅ 実機で確認済み（カメラ約30fps、推論約20回/秒） |
| FreeRTOS → μT-Kernel 互換レイヤー（`rtos_to_mtk.h`） | ✅ 実装済み |
| カメラ・AI 推論の μT-Kernel タスク化（2タスク構成） | ✅ 実装済み / 🔶 カメラ画像の受信まで実機で確認。LCD 表示は故障のため未確認 |
| FSP 設定の移植（VIN / MIPI CSI / GLCDC / D/AVE 2D / Ethos-U / I2C / GPT、ピン・クロック） | ✅ 実装済み |
| 転倒判定・アラート・プライバシー保護 UI・システム監視（6タスク構成） | 📝 設計済み（[docs/design.md](docs/design.md)）、代替機到着後に実装 |
| 検知率・処理遅延の評価 | 📝 代替機到着後に実施 |

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

```
 OV5640 ──MIPI CSI──▶ VIN ──▶ [camera task (prio 9)] ──イベントフラグ──▶ [AI task (prio 10)] ──▶ Ethos-U55
                                   │  RGB565→INT8 前処理                      │  YOLO 推論＋後処理
                                   ▼                                         ▼
                               GLCDC/DRW ◀──────────── 検出結果 ─────────────┘
                               (LCD 表示)
 ※ μT-Kernel のイベントフラグで FreeRTOS の EventGroup を置き換え
```

設計の詳細（計画中の 6 タスク構成・転倒判定アルゴリズムを含む）は [docs/design.md](docs/design.md) を参照してください。

---

## リポジトリ構成

```
.
├── mtk3bsp2_ra8p1_ek/          ★応募プログラム本体（e² studio プロジェクト）
│   ├── Application/app_main.c   usermain()：タスク・イベントフラグの生成
│   ├── src/rtos_to_mtk.h        FreeRTOS API → μT-Kernel API 互換レイヤー（自作）
│   ├── src/camera_display_thread_entry.c   カメラ取得・前処理・表示（移植）
│   ├── src/ai_inference_thread_entry.c     NPU 推論（移植）
│   ├── src/hal_warmstart.c      LCD パネルのリセット解除処理を追加
│   ├── src/ai_application/      推論・後処理・変換済みモデル（ルネサス/Arm/EdgeCortix 提供）
│   ├── src/camera_layer/, src/display_layer/   カメラ・LCD ドライバ層（ルネサス提供を改変）
│   ├── mtk3_bsp2/               μT-Kernel 3.0 BSP2（TRON フォーラム）
│   └── configuration.xml        FSP 設定
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

---

## ライセンスと使用している既存ソフトウェア

本リポジトリは**オープンソースとして公開**しています。

- 応募者が作成した部分（`src/rtos_to_mtk.h`、`Application/app_main.c` の追加部分、各ファイルの μT-Kernel 対応の改変、`docs/` 以下）: **MIT License**（[LICENSE](LICENSE)）
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

1. LCD 表示を含む μT-Kernel 版の実機動作確認
2. 6 タスク構成（カメラ／AI 推論／転倒判定／アラート／UI／システム監視）への拡張
3. 転倒判定（検出枠の縦横比・高さの時系列変化）の実装とパラメータ調整
4. 検知率・誤報率、「フレーム取得 → 推論 → 判定 → 通知」の遅延計測
5. 人物（person）検出モデルへの置き換え

## 作者

田中 真 — GitHub: [@syouun](https://github.com/syouun)
