#!/bin/bash

#Qtのフレームワークをコピーしてアプリケーション内に設置する
#このスクリプトと同じ場所に.appを作成する
cd $(dirname $0)
mkdir -p CaptureStream.app/Contents/Frameworks
cp -R ~/QtSDK/Desktop/Qt/4.8.0/gcc/lib/{QtCore,QtGui,QtNetwork,QtScript,QtXml,QtXmlPatterns}.framework CaptureStream.app/Contents/Frameworks

#ここで各フレームワーク内のファイル名にdebugが入っているファイルを消す
find CaptureStream.app/Contents/Frameworks -name \*debug\* -delete
#ここで各フレームワーク内のHeadersディレクトリとHeadersへのシンボリックリンクを消す
find CaptureStream.app/Contents/Frameworks -name Headers -delete

#フレームワークのid設定
install_name_tool -id @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore CaptureStream.app/Contents/Frameworks/QtCore.framework/Versions/4/QtCore
install_name_tool -id @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui CaptureStream.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
install_name_tool -id @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork CaptureStream.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork
install_name_tool -id @executable_path/../Frameworks/QtScript.framework/Versions/4/QtScript CaptureStream.app/Contents/Frameworks/QtScript.framework/Versions/4/QtScript
install_name_tool -id @executable_path/../Frameworks/QtXml.framework/Versions/4/QtXml CaptureStream.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml
install_name_tool -id @executable_path/../Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns CaptureStream.app/Contents/Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns

#各フレームワーク相互の依存の修正
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore CaptureStream.app/Contents/Frameworks/QtGui.framework/Versions/4/QtGui
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore CaptureStream.app/Contents/Frameworks/QtNetwork.framework/Versions/4/QtNetwork
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore CaptureStream.app/Contents/Frameworks/QtScript.framework/Versions/4/QtScript
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore CaptureStream.app/Contents/Frameworks/QtXml.framework/Versions/4/QtXml
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore CaptureStream.app/Contents/Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns
install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork CaptureStream.app/Contents/Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns
