﻿// pch.h: プリコンパイル済みヘッダー ファイルです。
// 次のファイルは、その後のビルドのビルド パフォーマンスを向上させるため 1 回だけコンパイルされます。
// コード補完や多くのコード参照機能などの IntelliSense パフォーマンスにも影響します。
// ただし、ここに一覧表示されているファイルは、ビルド間でいずれかが更新されると、すべてが再コンパイルされます。
// 頻繁に更新するファイルをここに追加しないでください。追加すると、パフォーマンス上の利点がなくなります。

#ifndef PCH_H
#define PCH_H
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4561)

// プリコンパイルするヘッダーをここに追加します
#include <Windows.h>
#include "YmBase/YmBase.h"
#include "YmTngn/YmTngnFwd.h"

#endif //PCH_H
