# Hagoromo

[![Hagoromo Demo](http://img.youtube.com/vi/k0RUaP7xOoo/0.jpg)](https://www.youtube.com/watch?v=k0RUaP7xOoo "Hagoromo Demo")

デモ URL:
　https://youtu.be/k0RUaP7xOoo?si=x3SaB8hpCM4vEO_E

　　- [0:00](https://youtu.be/k0RUaP7xOoo?si=EM5wYsA9CxxWqT2o) ～ プラグイン適用なし

　　- [1:39](https://youtu.be/k0RUaP7xOoo?si=H5prtSiqccYcSImJ&t=99) ～ プラグイン適用あり

　　　└ プラグイン適用なしの動画のとおり、スキニングデータはダンスモーションのみです

　　　└ それ以外は純粋に当該プラグインで計算した結果をアニメーションポーズとして出力しています

<br/>
<br/>

---

<br/>

## 特徴 :

<br/>

　**● Unreal Engine 上で動作する、ボーンを利用した位置ベースの物理シミュレーションプラグインです**

　**● Anim Node として実装しています**

　**● 位置ベースの手法により非常に安定したシミュレーション結果を出力します**

　**● アルゴリズムに XPBD を採用し、異なるフレームレート間でもほとんど同じ剛性を実現しています**

　**● アニメーションデータがある場合、アニメーションポーズをガイドラインとしたシミュレーションも可能です**

　**● 物理を調整するパラメータをボーン単位で細かくコントロールする機能を用意しています**

　**● 静的・動的なコライダに対応した堅牢な衝突検出システムを実装し、可能な限り貫通を防止しています**

　**● UE の Vector Register( SIMD )を利用した数学ライブラリを開発し、ほぼ全ての計算が高速に実行されます**

　**● プラグイン導入にあたりエンジン改造は不要です**

　**● 無料**

<br/>
<br/>

---

<br/>

## 導入方法 :

<br/>

　**▼ C++のビルド環境がある場合**

<br/>

　　　**Hagoromo** フォルダをプロジェクトの **Plugins** 以下に配置してビルドしてください。

<br/>
<br/>

　**▼ C++のビルド環境がない場合( ※UE5のみ対応 )**

<br/>

　　　**( 1 )**　**[Tags > 20250503_UE5.5_v1.0.0](https://github.com/nozoxa/Hagoromo/releases/tag/20250503_UE5.5_v1.0.0)** から **Hagoromo.zip** をダウンロードします。

　　　**( 2 )**　**Hagoromo.zip** を解凍します。

　　　**( 3 )**　**Hagoromo** フォルダをプロジェクトの **Plugins** 以下に配置します。

　　　**( 4 )**　エディタを起動します。

<br/>
<br/>

---

<br/>

## 使い方 :

<br/>

　基本的な使い方は [Wiki](https://github.com/nozoxa/Hagoromo/wiki/Hagoromo-%E3%81%AE%E4%BD%BF%E3%81%84%E6%96%B9) をご確認ください。

<br/>

　実際の設定例については [セットアップ例](https://github.com/nozoxa/Hagoromo/wiki/%E3%82%BB%E3%83%83%E3%83%88%E3%82%A2%E3%83%83%E3%83%97%E4%BE%8B) を参考にしていただけますと幸いです。

<br/>
<br/>

---

<br/>

## ライセンス :

<br/>

　MIT

<br/>
<br/>

---

<br/>

## 作者 :

<br/>

　[ノゾクサ@nozoxa_0131](https://x.com/nozoxa_0131)

<br/>

---

<br/>

## その他 :

<br/>

　**● [実装にあたり参考にした資料](https://github.com/nozoxa/Hagoromo/wiki/%E5%AE%9F%E8%A3%85%E3%81%AB%E3%81%82%E3%81%9F%E3%82%8A%E5%8F%82%E8%80%83%E3%81%AB%E3%81%97%E3%81%9F%E8%B3%87%E6%96%99
)**

<br/>
<br/>

---
