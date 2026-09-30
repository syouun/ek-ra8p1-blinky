# 動作確認手順書（EK-RA8P1 + μT-Kernel 3.0）

審査・評価される方が、ソースコードからビルドしてボードで動作を確認するための手順です。

> **提出時点の注意**: 開発中に貸与ボードの LCD が故障したため、μT-Kernel 版での **LCD 表示は未確認**です（README の「現在の開発状況」参照）。代替機での確認後、本書を更新します。

---

## 1. 必要なもの

| もの | 備考 |
|---|---|
| EK-RA8P1 キット一式 | ボード本体、グラフィックス拡張ボード＋7インチ LCD（1024×600）、カメラ拡張ボード＋OV5640 カメラ、FFC ケーブル、USB Type-C ケーブル（すべてキット同梱） |
| Windows 10/11 PC | e² studio を使用 |
| シリアルターミナル（任意） | Tera Term など。115200bps / 8N1 |

**追加のハードウェアは不要です。**

---

## 2. ハードウェアの準備

1. **電源を入れる前に**、グラフィックス拡張ボード（LCD）をピンヘッダ **J1** に、カメラ拡張ボード＋OV5640 を FFC ケーブルで取り付けます（EK-RA8P1 クイックスタートガイドの図のとおり）。
   - J1 のピンが 1 列ずれていないか、FFC の向き（接点面）が正しいかを確認してください。
2. **SW4 を 8 個すべて OFF** にします（SW4-6 OFF = パラレル LCD ＋ MIPI カメラ。公式 AI サンプルと同じ設定）。
3. その他のジャンパは出荷時設定のままにします（J16 = 2-3）。
4. **J10（USB Debug）** と PC を USB Type-C ケーブルで**直接**接続します（USB ハブは使わないでください。ケーブルの劣化で動作が不安定になった事例があります）。

---

## 3. 開発環境のインストール

1. [Renesas FSP Releases](https://github.com/renesas/fsp/releases) から **FSP v6.6.0 の Platform Installer**（`setup_fsp_v6_6_0_e2s_v2026-07.exe`）をダウンロードしてインストールします。
   - ツールチェーンは **GCC ARM Embedded 13.2.1** を使用します。J-Link ドライバも一緒に入ります。
2. e² studio を起動し、**日本語・空白を含まないパス**（例: `C:\work\ra8p1`）をワークスペースに指定します。

### 3.1 C++ ヘッダが見つからないエラーの回避（Windows のパス長制限）

本プロジェクトは推論の後処理に C++ を使用します。ツールチェーンが深い階層にインストールされていると、Windows のパス長制限（260 文字）により `bits/c++config.h: No such file or directory` などのエラーになることがあります。その場合は次のようにします。

1. 管理者のコマンドプロンプトで、短いパスのジャンクションを作ります。
   ```
   mklink /J C:\armgcc13 "<e² studio のツールチェーンフォルダ>\arm-gnu-toolchain-13.2.Rel1-mingw-w64-i686-arm-none-eabi"
   ```
2. e² studio の「ヘルプ → Renesas ツールチェーンの追加」で `C:\armgcc13` を登録し、元の長いパス側のチェックを外します。

詳しくは [porting_notes.md の 11 章](porting_notes.md) を参照してください。

---

## 4. ビルド

1. このリポジトリを取得します。
   ```
   git clone https://github.com/syouun/ek-ra8p1-blinky.git
   ```
   （Git を使わない場合は GitHub の「Code → Download ZIP」で取得して展開）
2. e² studio で `File` → `Import` → `General` → `Existing Projects into Workspace` を選び、取得したフォルダ内の **`mtk3bsp2_ra8p1_ek`** を指定して `Finish`。
3. プロジェクト内の **`configuration.xml`** をダブルクリック → 右上の **`Generate Project Content`** をクリック。
4. `Project` → `Build Project`（トンカチアイコン）。`Build Finished. 0 errors` を確認します。
   - ビルドが終わったら `Debug/mtk3bsp2_ra8p1_ek.elf` の更新時刻が新しくなっていることを確認してください。

> μT-Kernel 3.0 BSP2（`mtk3_bsp2/`）はプロジェクト内に同梱しているため、別途ダウンロードは不要です。

---

## 5. 書き込みと実行

1. `Run` → `Debug Configurations...` → `Renesas GDB Hardware Debugging` → **`mtk3bsp2_ra8p1_ek Debug_Flat`** を選び `Debug`。
   - 「プログラム・ファイルが存在しません」と出る場合は、`Main` タブの C/C++ Application に `Debug/mtk3bsp2_ra8p1_ek.elf` を指定してください。
   - J-Link の接続が不安定な場合は、`Debugger` タブの接続速度を 4000kHz → 1000kHz に下げてください。
2. `main` などで一時停止したら **Resume（F8）** を押します（停止位置によっては 2 回）。
3. 以前のデバッグで設定したブレークポイントが残っていると途中で止まります。`Run` → `すべてのブレークポイントを削除` を実行してください。

---

## 6. 期待される動作

| 確認項目 | 期待される結果 |
|---|---|
| μT-Kernel の起動 | シリアルターミナル（μT-Kernel の T-Monitor 出力：SCI8＝PD02/PD03、115200bps。通常は J-Link の仮想 COM ポートで受信できます）に `Start User-main program.` と表示される |
| マルチタスク動作 | LED1（青）・LED2（緑）・LED3（赤）がそれぞれ 500ms / 700ms / 1000ms 周期で点滅し、`task 1` `task 2` `task 3` が出力される |
| カメラ・AI 推論 | カメラタスク（優先度 9）と AI 推論タスク（優先度 10）が起動し、カメラ画像の取得と NPU 推論が繰り返される |
| LCD 表示 | カメラ映像と顔検出の枠が LCD に表示される（**※故障のため μT-Kernel 版では未確認**） |

### 動作状況をデバッガで確認する方法

- イベントフラグの状態: Debugger Console で `p g_ai_app_event` でフラグ ID を確認し、Expressions に登録して値の変化を見る
- カメラ・LCD の割り込みやフレームバッファの確認方法は [porting_notes.md の 15・17 章](porting_notes.md) を参照

---

## 7. トラブルシューティング

| 症状 | 確認すること |
|---|---|
| LCD が真っ白／真っ黒 | 拡張ボードの J1 挿し込み、SW4 がすべて OFF か、USB ケーブルを良品に交換して PC に直挿し。公式の工場出荷時デモでも映らない場合はハード故障の可能性 |
| カメラの初期化エラー（`FSP_ERR_HW_LOCKED`） | カメラ FFC の向き、カメラリセット（P709）、カメラクロック（GPT ch12 / P501 / 24MHz） |
| 初期化は成功するが画像が来ない | P108（MIPI_IF_EN）が GPIO 出力 High になっているか、BSP の SDRAM Support が有効か |
| ビルドエラー（C++ 標準ヘッダが見つからない） | 3.1 のパス長対策 |
| `configuration.xml` の部品が二重に表示される | e² studio を保存せずに終了し、`ra_gen` の中身を削除してから再度 Generate |

その他の既知の問題と解決方法は [porting_notes.md](porting_notes.md) にまとめています。
