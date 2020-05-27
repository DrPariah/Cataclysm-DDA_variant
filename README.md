
# Cataclysm-DDA_variant_jp

English version (broken english warning!!)

[https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/README_en.md](https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/README_en.md)

## これはなに？

Cataclysm DDAを独自改造したバージョン、いわゆるヴァリアントになります。
基本的に、安定版0.E(#10478)がベースです。

## 注意

- 人によって好みが分かれる要素がありますが、オプションから無効にできるようにしてます。
  - 排泄要素オプション。
  - 異界のメイドさんとの\*Lovely Activity\*を追加するmod。
  - サキュバス等のHentai要素を追加するmod。

## ダウンロードと遊び方

- **[ダウンロードページ](https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/DOWNLOAD.md)を作りました。**
  - [https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/DOWNLOAD.md](https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/DOWNLOAD.md)

- ヴァリアント本体に同梱されているもの
	- ヴァリアント本体
		- [Googleドライブ](https://drive.google.com/drive/u/0/folders/1f6kz2fS5UWXGt9e1AZ6fnzigVi6Ligx7)で配布しています。
		    - バージョンの「r〇〇」という数字が大きいほど、新しいビルドです。
            - **基本的に、バージョン名が「0.E_r〇〇」のみで、以下のような接尾語を付かないものを選んでください。**
                - latest_experimentalと付くものは、開発版をベースして改造したものです。
                - prereleaseと付くものは、不安定な機能を試験的に公開しているものです。
                - 32bitと付くものは、32bit用ビルドです。32bitのOSを使っている方向けです。
                - androidと付くものは、android用のapkです、インストール方法は各自でお調べお願い致します。
        - 予備として、こちらの[アップローダー](https://ux.getuploader.com/cdda_variant_jp/)でも配布していますが、更新が遅いことが多いです。
	- Battle Maid Mod(現バージョン対応済み)
		- 改変、再配布は大丈夫との事だそうなので、これも同梱させて頂いています。
		- アドオンとしても常識の範囲をはみ出してるのは自覚してますが、あくまでアドオンなのでで通りますでしょうか...？

- 各自ご用意いただきたい物
	- UnDeadPeopleタイルセット
		- なくてもプレイにそこまで支障はないですが、タイルはこれのみ対応なので是非。

- 翻訳について
	- 日本語_variantという言語を同梱し、オプションから選べるようにしています。
	- 新要素の翻訳はこちらにしか入っていないので、オプションから言語の変更をお願いします。

- 同梱modについて
  - コア - Variant Addition
    - ヴァリアントに必須のファイルです。
  - コア - LittleMaid
    - 愛玩用リトルメイド(搾乳可能)から召喚可能なリトルメイドをクラフトできるようにします。
    - ショゴスのメイドさんに対してヴァリアントの機能を使えるようにします。
    - メイド要素がいらない人は、Battle Maidともども入れなくても通常のプレイには問題ないはずです。
  - 追加 - ホードビーコン
    - ゾンビの襲撃を任意に起こすアイテムを追加します。
    - ヴァリアント専用modですが、使い方によっては強力すぎるアイテムのため、導入しない選択肢のために分離しました。
	- 追加 - FTLビーコン自然生成
	  - FTLドライブのワープ先となるFTLビーコンが、道路脇に自然生成されるようになります。
	  - FTLビーコンは、自分で設置する事もできますので、FTLドライブをより使いやすくするための追加modです。
  - 追加 - Vibrator Junkyard
		- こまごまとした様々なアイテムを追加します。ほとんどが名前の通りJunkでヴァリアントとはあまり関係ないオマケアイテムです。
  - 追加 - Hentai Mod Reloaded
		- サキュバスなどのモンスターを初めとしたHENTAI的な要素を追加します。詳しくはmodファイル内のドキュメントを見てください。

## 新要素

新要素の詳細は以下のファイルにまとめました。

[https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/doc/VARIANT_FEATURE.md](https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/doc/VARIANT_FEATURE.md)

代表的な新要素の簡単な紹介です。

### うんちを出そう

@さんが食事を消化するにしたがって、腸内に何かがたまってきます。

もよおしてきたら、ちょっとゾンビに邪魔されない所でかがんで、出すものを出しましょう。

出すべきところで出せば、さらなる意欲アップにつながります。

人によってはいらないジャマな要素になるので、世界設定のオプションからオフにもできます。

### シャワーを浴びよう

車載シャワーブースを作って車に搭載すれば、いつでもシャワーを浴びられます。

シャワーを浴びるためには車載タンクの綺麗な水30リットルと自動車用バッテリー1個分の電気をを消費します。

### メイドともっと楽しもう

Battle Maid Mod内ではアイテムだけの存在だった愛玩用リトルメイドを、友好モンスターとして召喚したり逆に片付けたりできます。

また、ショゴスのメイドさんや愛玩用リトルメイドに対して待機命令を出したり、一緒に遊んだりできます。

### 酸性雨には気を付けよう

デフォルトの設定だと、まれに酸性雨が降ります。世界設定のオプションから調整できます。

酸性雨を浴びると多大な苦痛と少量のダメージを受けます。レインコートを着たり、傘を構えることで被害を大幅、あるいは完全に軽減できます。

まれに、非常に強力な酸性嵐 [Acid Storm] に発展します。もはや傘では防げないレベルなので、屋根のあるところに避難しましょう。

冬には、酸性雨と小酸性雨のかわりに、酸性雪と小酸性雪が降ります。効果はほぼ同じです。

### FTL: Fast TraveL

FTLとは、Fast TraveLの略語です、光る？

未来的な部品を集めて、特殊なFTLエンジンを製作して車両に搭載することで、

原子力電池1個と引き換えに、あらかじめFTLビーコンを設置しておいた場所へ車両と乗員ごとワープ移動する、FTLドライブができるようになります。

細かい手順や製作条件は全要素解説の方で説明します。

### etc...

ここでは紹介しきれない様々な微調整もあります。

こちらで全要素を紹介しています。

[https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/doc/VARIANT_FEATURE.md](https://github.com/roloa/Cataclysm-DDA_variant/blob/variant_jp/doc/VARIANT_FEATURE.md)


## 謝辞（謝罪）

- Kevin Granadeさま、ならびにCDDAコントリビューターの皆さま
- Battle Maid Modの作者さま
- UnDeadPeopleタイルセットに入ってるメイド関連のチップの作者さま

以上の皆様に感謝とともに非礼をお詫びいたします。

こんなヴァリアントつくってすみません。でも作りたかったんです。

## 主な共同開発者について

  - lispcoc < https://github.com/lispcoc >
  - roloa (リポジトリオーナー) < https://github.com/roloa >

(敬称略)

このヴァリアントには以上の開発者によってつくられました。

みなさまのご協力に大変感謝します。

## ライセンスについて

CDDA本家と同じ、クリエイティブコモンズの 表示 - 継承 3.0 非移植 ライセンスとします。

詳しくは以下のファイルを見てください。

https://github.com/roloa/Cataclysm-DDA/blob/variant_sca/LICENSE.txt

このヴァリアントの配布物には、以下の制作物を同梱・利用させて頂いております。

- Cataclysm:DDA翻訳ファイル(C:DDAフォーラム内の色々なMOD翻訳文追加版)
  - https://drive.google.com/drive/u/2/folders/1bEPMkwqzgxiqlzOWhdMckU9h-6pAaP7O
  
- (latest_experimentalビルドのみ) BattleMaidMod#10481開発版対応
  - https://ux.getuploader.com/cataclysm_jp/download/47

## Discord

[https://discord.gg/kGNU8Jg](https://discord.gg/kGNU8Jg)

サーバー参加時のランダムメッセージは切ってあるのでご気軽にご参加して、こっそり脱退してみてください。

不具合報告や、要望、ご意見、ご感想などお待ちしております。

