# 動作確認手順書（EK-RA8P1 + μT-Kernel 3.0）

審査・評価される方が、ソースコードからビルドしてボードで動作を確認するための手順です。

> **注意**: 開発中に貸与ボードの LCD が故障したため、6 タスク版の**実機での動作は代替機の到着後に確認**します（ビルドと転倒判定ロジックの単体テストは確認済み。README の「現在の開発状況」参照）。確認後、本書を更新します。
>
> 応募締切時点の提出版を確認する場合は `submission-2026-09-30` ブランチ、6 タスク版は `develop` ブランチを使ってください（`git checkout develop`）。

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

## 6. 期待される動作（6 タスク版）

シリアルターミナル：μT-Kernel の T-Monitor 出力（SCI8＝PD02/PD03、115200bps、8N1）。通常は J-Link の仮想 COM ポートで受信できます。

| 確認項目 | 期待される結果 |
|---|---|
| 起動 | `=== Privacy-first fall detection edge AI ... ===` の後に `[MAIN] task alert started (id .., priority 5)` など 6 タスクの起動ログが出る |
| LED | 緑（LED2）点灯＝見守り中。青（LED1）は AI 推論中に点滅 |
| LCD（誰もいない） | 画面全体が**緑**で `SAFE`、中央の四角（カメラの視野を表す）は空。**カメラ映像は表示されない** |
| LCD（人が映る） | **黄**で `MONITORING`、中央の四角に顔の位置が白い枠で表示される |
| 急に低い位置へ移動 | **橙**で `CHECKING`、シリアルに `[JUDGE] suspect: sudden drop` |
| そのまま 3 秒 | **赤**で `FALL DETECTED`、**赤 LED（LED3）点灯**、シリアルに `[ALERT] ***** FALL DETECTED ***** (#1, frame capture -> LED xx ms ...)` |
| 起き上がって 1 秒 | 緑 LED に戻り、シリアルに `[ALERT] recovered` |
| 監視 | 10 秒ごとに `[SYSMON] up ..s frames .. inferences .. judged .. | state .. person .. falls ..` |

### 安全な確認方法

実際に人が倒れる必要はありません。カメラを人の顔の高さ付近に向け、**顔が映った状態から素早く（1 秒以内に）画面の下 4 割へ移動し、3 秒とどまる**と転倒と判定されます。タブレットに転倒の動画や顔写真を表示してカメラの前で動かす方法でも確認できます。判定のしきい値は `src/app_config.h` の `APP_FJ_*` で変更できます。

### 転倒判定ロジックだけを PC で確認する

ボードがなくても、判定ロジックの単体テストを PC で実行できます（gcc または clang が必要）。

```
cd tests
make
```

`all tests passed` と表示されれば正常です。

### 元サンプルの画面（カメラ映像）で確認したい場合

デバッグ用に、`src/app_config.h` の `APP_PRIVACY_UI_ENABLE` を `0` にしてビルドすると、元のルネサスサンプルと同じ「カメラ映像＋顔の枠」の画面になります（この場合 UI タスクは起動しません）。

### 動作状況をデバッガで確認する方法

- イベントフラグの状態: Debugger Console で `p g_ai_app_event` でフラグ ID を確認し、Expressions に登録して値の変化を見る
- カメラ・LCD の割り込みやフレームバッファの確認方法は [porting_notes.md の 15・17 章](porting_notes.md) を参照

---

## 7. トラブルシューティング

| 症状 | 確認すること |
|---|---|
| LCD が真っ白／真っ黒 | 拡張ボードの J1 挿し込み、SW4 がすべて OFF か、USB ケーブルを良品に交換して PC に直挿し。公式の工場出荷時デモでも映らない場合はハード故障の可能性 |
| カメラの初期化エラー（`FSP_ERR_HW_LOCKED`） | カメラ FFC の向き、カメラリセット（P709）、カメラクロック（GPT ch12 / P501 / 24MHz） |
| 初期化は成功するが画像が来ない | P108（MIPI_IF_EN）が GPIO 出力になっているか（Low で有効）、BSP の SDRAM Support が有効か |
| ビルドエラー（C++ 標準ヘッダが見つからない） | 3.1 のパス長対策 |
| `configuration.xml` の部品が二重に表示される | e² studio を保存せずに終了し、`ra_gen` の中身を削除してから再度 Generate |

その他の既知の問題と解決方法は [porting_notes.md](porting_notes.md) にまとめています。
