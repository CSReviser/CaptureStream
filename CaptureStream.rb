#!/usr/bin/env ruby
# encoding: UTF-8

require 'open-uri'
require 'rexml/document'
require 'kconv'
require 'nkf'
require 'date'
require 'tempfile'
require 'fileutils'

=begin

＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝【注意事項】＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
このスクリプトの使用結果については作者は責任を負うことはできません。できる限りのテストを行い、
善意を持って作成しておりますが、すべて使用される方の自己責任でお願いいたします。

＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝【更新履歴】＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
2020/08/21 2020年度の講座に合わせてURLを変更。
2020/04/10 2020年度の講座に合わせて、レベルアップ中国語／ハングルを削除。
2019/04/06 ボキャブライダーのみ今年度を指定する機能追加。
2019/01/03　2018年度の講座に合わせて、追加、削除。デフォルト拡張子を”mp3”に変更。
2014/04/07　不要なコードを削除。「エンジョイ・シンプル・イングリッシュ」対応。
2014/04/06　Rubyの対応バージョンを2.0.0以降に変更。2014/03/31の仕様変更に対応。
			「ニュースで英会話」と「ABCニュースシャワー」を削除。
2013/04/10　flvstreamerのダウンロード場所を更新。ffmpegのダウンロードに関する記述を追加。
2013/04/09　「英語で読む村上春樹」対応。
2013/04/08　2013年度対応版。
　　　　　　　gnash対応コード削除。scramble.xmlのuri変更。
2012/04/09　「リトル・チャロ」削除。「英会話タイムトライアル」、「まいにちロシア語」、
　　　　　　　「レベルアップ中国語」、「レベルアップハングル講座」に対応。
2011/10/03　「攻略！英語リスニング」対応。
2011/07/13　「ニュースで英会話」の公開中のファイルのダウンロードをenewsとし、過去分すべての
　　　　　　　ダウンロードをenews-allに変更。
2011/07/11　翌週公開分ダウンロードのコードを削除。作業ファイル名の生成方法を変更(make_temp_name)
2011/07/06　xmlの取得元をwikiに再変更。
2011/07/02　デフォルトの動作を基礎英語１のダウンロードから何もダウンロードしないように変更。
　　　　　　　flvstreamerの実行をカレントディレクトリからパスの通ったものに変更。
　　　　　　　flvstreamerが実行できなかった場合にエラーメッセージを出して終了するように修正。
2011/07/01　githubにリポジトリ作成。xmlの取得元をgithubに変更。
2011/06/27　flare対応削除。wikiから日付指定でコードを取得するように修正。Windows版sdl-gnash
　　　　　　　に対応。独自ビルドのMac版sdl-gnashでの動作確認済み。
2011/05/02　streaming.swf内のActionScriptの変更に暫定対応。
2011/04/18　flare対応。
2011/04/18　gnash対応。
2011/04/13　scramble.xmlのダウンロード場所を変更。
2011/04/13　「ABCニュースシャワー」に対応。スクランブル文字列をウィキから自動取得するように修正。
　　　　　　　翌週公開分のダウンロードをコメントアウト。
2011/04/05　ストリーミングのURLに追加された文字列への緊急対応。
2010/05/12　ゴールデンウィーク対応。flvファイル名に追加される'mm'と'vip'に対応。
2010/04/22　「ニュースで英会話」に対応。指定可能な引数にallを追加。
2010/04/21　「リトル・チャロ2」に対応。ffmpegへの依存をなくし、独自にflvからmp3を抽出。
2010/04/20　実践ビジネス英語の10-ebj-4231-295vip.flvの形式に対応。複数講座ダウンロード対応。
2010/04/11　翌週公開ファイルに対応。id3タグのアルバム名のデフォルトを「講座名_YYYY_MM_DD」に
　　　　　　　変更。
2010/04/10　コードの整理。スクリプトが存在するディレクトリ以外の場所から相対パスで実行された
　　　　　　　場合に対応。
2010/04/05　プログラムが先祖返りしてrtmpdumpを使うようになっていたのをflvstreamerを使うよう
　　　　　　　に修正。新年度最初の３月中の放送分が翌年扱いになっていたのを修正。
2010/04/04　放送年の扱いをすべて年度から歴年に変更。id3タグのアルバムとタイトルをカスタマイ
　　　　　　　ズ可能に。mp3のファイル名を「講座名_YYYY_MM_DD.mp3」の形式に変更。id3タグのアル
　　　　　　　バム名のデフォルトを「YYYY_MM_DD」に変更。

＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝【　準備　】＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
このスクリプトを実行するためにはRubyとffmpegが必要です。ffmpegはパスの通ったフォルダに存在
する必要があります。

以下のURLまたは適当な場所からダウンロードするか、ご自分でビルド／インストールしてください。
以下のURLはあくまで参考のために載せているだけで、リンク先のバイナリの内容については責任を持
てません。ご自身で判断の上、お使いください。

【Windows環境】
・Ruby
	下記のRubyInstaller for Windowsのサイトから2.0.0以降の最新版をインストールしてください。
		http://rubyinstaller.org/downloads/
	このドキュメントを記述している時の最新版のリンクです。
		http://dl.bintray.com/oneclick/rubyinstaller/rubyinstaller-2.0.0-p451.exe?direct
・ffmpeg
	http://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-20140403-git-fd2bcfc-win32-static.7z
	上記のファイルをブラウザでダウンロードして解凍した後、ファイル名をffmpeg.exeに変更し、
	パスの通っているフォルダに置いてください。Windowsの場合は、CaptureStream.rbと同じフォ
	ルダでもOKです。ffmpeg-20140407-git-a7a82f2-win32-staticではHLSのダウンロードに失
	敗しますので、必ずしも最新のffmpegがよいとは限りません。
	拡張子の7z(7-Zip)については以下のurlを参照してください。
		http://www.7-zip.org/				英語
		http://sevenzip.sourceforge.jp/		日本語

【Macintosh環境】
・Ruby
	標準でインストールRubyのバージョンが1.8.7の場合、自分で2.0.0以降をインストールする必要
	があります。パッケージ管理システムのHomebrewでrbenvとruby-buildをインストールし、rbenv
	でrubyをインストールするのがお勧めです。
	以下のurlが参考になります。10.8以下のシステムでも手順はそのままで大丈夫でしょう。
		Mac OS X 10.9にrbenvを使って複数バージョンのRubyをインストールする - Qiita
		http://qiita.com/ryam/items/33803f9a442399b60232
・ffmpeg
	http://www.evermeet.cx/ffmpeg/ffmpeg-2.2.7z		（もしくは最新版）
	上記のファイルをブラウザでダウンロードして解凍した後、ファイル名をffmpegに変更し、ター
	ミナルで実行属性を付け、パスの通ったディレクトリに置いてください。このバイナリはx86_64
	のため、i386またはppcバイナリが必要な場合はソースコードからビルドするか、パッケージマ
	ネージャ(Homebrew、MacPorts、Fink)でインストールしてください。Mac版のCaptureStream
	最新版に含まれているものを利用することも可能です。

【Linux環境】
・Ruby
	パッケージでrbenvをインストールし、rbenvでrubyをインストールするのがお勧めです。
・ffmpeg
	http://ffmpeg.org/download.html
	上記のページの「FFmpeg Linux Builds」セクションからリンクをたどって最適なものをダウンロード
	してパスの通ったディレクトリに置くか、パッケージマネージャを使用してインストールしてください。
	
＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝【使用方法]＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
・Windowsの場合
	DOSプロンプトを開き、このスクリプトの入っているフォルダに移動して「ruby CaptureStream.rb 講座名」
	を実行します。もしくは、下記に従ってデフォルトの動作を設定してあれば、CaptureStream.rb
	をダブルクリックしても実行可能のはずです。

・MacintoshおよびLinuxの場合
	ターミナルを起動し、このスクリプトの入っているフォルダに移動して「ruby CaptureStream.rb 講座名」
	を実行します。

講座名のところには以下のものが複数指定可能です。allを指定するとすべての講座をダウンロードします。

basic1 basic2 basic3 timetrial kaiwa business1 business2 kouryaku yomu 
chinese levelup_chinese french italian hangeul levelup_hangeul german spanish russian
all

$default_target（配列）に指定しておくことで引数指定なしでダウンロードさせることができます。
入門ビジネス英語と実践ビジネス英語を指定するには以下のように設定します。

	$default_target = ['business1', 'business2']

=end

#--------------------------------------------------------------------------------
# 講座名リスト
#--------------------------------------------------------------------------------

$default_target = []
$english = %w!basic0 basic1 basic2 basic3 timetrial kaiwa business1 business2 gendai gakusyu enjoy everybody vr-radio!
$multilingual = %w!chinese omotenashi_chinese french french2 italian italian2 hangeul omotenashi_hangeul german german2 spanish spanish2 russian russian2!

#--------------------------------------------------------------------------------
# 今年度：ボキャブライダーのDL対象年度指定。ボキャブライダーのみ機能します。
#--------------------------------------------------------------------------------
$kon_nendo = '2020'

#--------------------------------------------------------------------------------
# 実行環境の検出とツールのパス設定
#--------------------------------------------------------------------------------

$is_windows = RUBY_PLATFORM.downcase =~ /mswin(?!ce)|mingw|cygwin|bccwin/
$script_path = File.expand_path( __FILE__ )
$ffmpeg = 'ffmpeg'

if $is_windows
	$script_path = $script_path.kconv( Kconv::UTF8, Kconv::SJIS )
	$null = 'nul'
else
	$null = '/dev/null'
end

#--------------------------------------------------------------------------------
#
#--------------------------------------------------------------------------------

def to_native( string )
	return $is_windows ? string.kconv( Kconv::SJIS, Kconv::UTF8 ) : string
end

def jputs( string )
	puts( to_native( string ) )
end

#--------------------------------------------------------------------------------
# flvファイルのサーバとオプション
#--------------------------------------------------------------------------------

$flv_host = 'flv.nhk.or.jp'
$flv_app = 'ondemand/'
$flv_service_prefix = 'mp4:flv/gogaku/streaming/mp4'

#--------------------------------------------------------------------------------
# 出力フォルダ名とmp3ファイル名の設定
# 以下は基礎英語1を例にして書いてありますが、他の講座は適宜読み替えてください。
# %H %K %F は以下のURLにあるXMLに含まれる属性の指定方法です。
# http://cgi2.nhk.or.jp/gogaku/st/xml/english/basic1/listdataflv.xml
# %k  XMLの講座名属性「基礎英語1」
# %h  XMLの放送日属性「4月XX日放送分」
# %f  XMLのflvファイル名属性から拡張子を除いたもの「09-ek1-4252-086」
# %r  このスクリプトの入っているフォルダ
# %p  パス（ディレクトリ）区切り文字（OSXでは/、Windowsでは\）
# %Y  ４桁の放送年（2010/04/04版で年度から年に変更）
# %y  ２桁の放送年（2010/04/04版で年度から年に変更）
# %M  ２桁の放送月(01~12)
# %m  放送月(1~12)
# %D  ２桁の放送日(01~31)
# %d  放送日(1~31)
# 上記以外の文字に%がついていた場合はその文字そのものとみなします。それ以外の文字はもちろんそのままです。
# フォルダ名の例： '%r%p英語%y%M' -> （このスクリプトがあるフォルダ内の）英語0905
# ファイル名の例： 'DE%Y%M%D' -> DE20090420
# 拡張子は$audio_extensionで指定します。
#--------------------------------------------------------------------------------

# 保存フォルダ名
$out_folder_hash = Hash.new( '%r%p%k' )

# 保存ファイル名
$out_file_hash = Hash.new( '%k_%Y_%M_%D' )

# id3タグのalbum
$id3_album = Hash.new( '%k' )

# id3タグのtitle
$id3_title = Hash.new( '%k_%Y_%M_%D' )

#--------------------------------------------------------------------------------
# 出力ファルが存在する場合にダウンロードをスキップする場合はtrueを、しない場合はfalseを設定
#--------------------------------------------------------------------------------

$skip_existing = true

#--------------------------------------------------------------------------------
# 音声ファイルの変換後の拡張子を設定（aacを指定するとダウンロードしたファイルそのもの）
# 3g2, 3gp, aac, avi, m4a, mka, mkv, mov, mp3, ts
#--------------------------------------------------------------------------------

$audio_extension = 'mp3'

#--------------------------------------------------------------------------------
# 音声ファイルの拡張子に対応したffmpegの実行コマンド
# 3g2, 3gp, aac, avi, m4a, mka, mkv, mov, mp3, ts
#--------------------------------------------------------------------------------
#$malformed = %w!3g2 3gp m4a mov!
$has3args = %w!3g2 3gp aac ts!
$akamai = 'https://nhks-vh.akamaihd.net/i/gogaku-stream/mp4/'
$ffmpeg_hash = {
	'3g2' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -vn -bsf aac_adtstoasc -acodec copy \"%s\"",
	'3gp' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -vn -bsf aac_adtstoasc -acodec copy \"%s\"",
	'aac' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -vn -acodec copy \"%s\"",
	'avi' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -id3v2_version 3 -metadata title=\"%s\" -metadata artist=\"NHK\" -metadata album=\"%s\" -metadata date=\"%s\" -metadata genre=\"Speech\" -vn -acodec copy \"%s\"",
	'm4a' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -id3v2_version 3 -metadata title=\"%s\" -metadata artist=\"NHK\" -metadata album=\"%s\" -metadata date=\"%s\" -metadata genre=\"Speech\" -vn -bsf aac_adtstoasc -acodec copy \"%s\"",
	'mka' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -id3v2_version 3 -metadata title=\"%s\" -metadata artist=\"NHK\" -metadata album=\"%s\" -metadata date=\"%s\" -metadata genre=\"Speech\" -vn -acodec copy \"%s\"",
	'mkv' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -id3v2_version 3 -metadata title=\"%s\" -metadata artist=\"NHK\" -metadata album=\"%s\" -metadata date=\"%s\" -metadata genre=\"Speech\" -vn -acodec copy \"%s\"",
	'mov' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -id3v2_version 3 -metadata title=\"%s\" -metadata artist=\"NHK\" -metadata album=\"%s\" -metadata date=\"%s\" -metadata genre=\"Speech\" -vn -bsf aac_adtstoasc -acodec copy \"%s\"",
	'mp3' => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -id3v2_version 3 -metadata title=\"%s\" -metadata artist=\"NHK\" -metadata album=\"%s\" -metadata date=\"%s\" -metadata genre=\"Speech\" -vn -acodec libmp3lame \"%s\"",
	'ts'  => "\"%s\" -y -i #{$akamai}%s/master.m3u8 -vn -acodec copy \"%s\""
}

#--------------------------------------------------------------------------------
# 出力フォルダ名と出力ファイル名のフォーマット文字列の解釈
#--------------------------------------------------------------------------------

# 引数はxmlのhdateとfileアトリビュート
def make_date( hdate, file )
	hdate =~ /^(\d+)\D+(\d+)/
	raise 'XMLに含まれている放送日の形式が違います。' if !$1 || $1.length < 1 || $1.length > 2 || !$2 || $2.length < 1 || $2.length > 2
	month = $1
	day = $2
	
	year = 2000 + file[0,2].to_i
	year += 1 if month =~ /^[123]$/ && Date.today.year > year
	return Date.new( year, month.to_i, day.to_i )
end

def format_name( format, target, kouza, hdate, file )
	result = String.new()
	
	hdate =~ /^(\d+)\D+(\d+)/
	raise 'XMLに含まれている放送日の形式が違います。' if !$1 || $1.length < 1 || $1.length > 2 || !$2 || $2.length < 1 || $2.length > 2
	month = $1
	day = $2
	
	year = 2000 + file[0,2].to_i
	year += 1 if month =~ /^[123]$/ && Date.today.year > year
	file = File.basename( file )	# ニュースで英会話が'201004/23_Fri/el/video/20100423_PULITZER'の形式のため
	
	chars = format.each_char.to_a
	percent = false
	i = 0
	while i < chars.size
		if percent
			percent = false
			case chars[i]
			when 'k'
				result << kouza
			when 'h'
				result << hdate
			when 'f'
				if file =~ /(.*)\.flv$/
					result << $1
				else
					result << file
				end
			when 'r'
				result << File.dirname( $script_path )
			when 'p'
				result << File::Separator
			when 'Y'
				result << year.to_s
			when 'y'
				result << (year % 100).to_s
			when 'M'
				result << '0' if month.length < 2
				result << month
			when 'm'
				result << month
			when 'D'
				result << '0' if day.length < 2
				result << day
			when 'd'
				result = day
			else
				result << chars[i]
			end
		else
			if chars[i] == '%'
				percent = true
			else
				result << chars[i]
			end
		end
		i += 1
	end
	
	return result
end

#--------------------------------------------------------------------------------
# 出力フォルダのチェックと作成
#--------------------------------------------------------------------------------

def check_output_dir( dir_path )
	result = false
	native_path = to_native( dir_path )
	
	if File.exist?( native_path )
		dir_info = File::Stat.new( native_path )
		if !dir_info.directory?
			jputs( "「#{dir_path}」が存在しますが、フォルダではありません。" )
		elsif !dir_info.writable?
			jputs( "「#{dir_path}」フォルダが書き込み可能ではありません。" )
		else
			result = true
		end
	else
		if !Dir.mkdir( native_path )
			jputs( "「#{dir_path}」フォルダの作成に失敗しました。" )
		else
			result = true;
		end
	end

	return result;
end

#--------------------------------------------------------------------------------
# 音声ファイルのダウンロード
#--------------------------------------------------------------------------------

def capture_stream( target, kouza, hdate, file, nendo )
	out_folder = format_name( $out_folder_hash[target], target, kouza, hdate, file ) # 出力フォルダ
	out_file = format_name( $out_file_hash[target] + '.' + $audio_extension, target, kouza, hdate, file ) # 音声ファイル（拡張子なし）
	id3_album = format_name( $id3_album[target], target, kouza, hdate, file )
	id3_title = format_name( $id3_title[target], target, kouza, hdate, file )
	
	#既存ファイル／フォルダのチェックのために、フォルダ名の末尾のセパレータがあれば取り除く
	parts = File.split( out_folder )
	out_folder = parts[0] + File::Separator + parts[1]
	
	exit unless check_output_dir( out_folder )
	out_folder += File::Separator
	
	out_folder = to_native( out_folder )
	out_file = to_native( out_file )
	id3_album = to_native( id3_album )
	id3_title = to_native( id3_title )

	if kouza == 'ボキャブライダー' 
		if !(nendo == $kon_nendo) 
				return true
		end
	end
	
	if $skip_existing && File.exists?( "#{out_folder}#{out_file}" )
		print( '-' )
		return true
	end
	
	result = false
	if file =~ /(.*)\.mp4$/ || file =~ /(.*)\.flv$/
		command = $ffmpeg_hash[$audio_extension]
		if $has3args.include?( $audio_extension )
			command = command % [$ffmpeg, file, out_folder + out_file]
		else
			command = command % [$ffmpeg, file, id3_title, id3_album, '20' + file[0..1], out_folder + out_file]
		end
		system( command + " > #{$null} 2>&1" )
		if $? == 0
			result = true
			print( 'O' )
		else
			print( 'X' )
			jputs( "\nffmpegの実行でエラーが起こりました。パスの通った実行可能な場所にHTTP Live Streaming(HLS)に対応したffmpegがあることを確認してください。" )
			File.unlink( out_folder + out_file ) rescue false
			exit
		end
	end
	
	return result
end

#--------------------------------------------------------------------------------
# メインプログラム
#--------------------------------------------------------------------------------

jputs( '語学講座ダウンローダ (2019/04/06)' )

Dir.chdir( to_native( File.dirname( $script_path ) ) )
targets = ARGV.length > 0 ? ARGV : $default_target
targets = $english + $multilingual if targets.include?( 'all' )

targets.each { |target|
	if !$english.include?( target ) && !$multilingual.include?( target )
		jputs( "サポートされていない講座名です：#{target}" )
		jputs( "使用方法: ruby #$PROGRAM_NAME [#{$english.join('|')}|#{$multilingual.join('|')}|all]" )
		exit
	end
}

targets.each { |target|
	if $english.include?( target )
		xml_uri = "http://cgi2.nhk.or.jp/gogaku/st/xml/english/#{target}/listdataflv.xml"
	elsif $multilingual.include?( target )
		if target =~ /^levelup_(.*)/
			xml_uri = "http://cgi2.nhk.or.jp/gogaku/st/xml/#{$~[1]}/levelup/listdataflv.xml"
		else
			xml_uri = "http://cgi2.nhk.or.jp/gogaku/st/xml/#{target}/kouza/listdataflv.xml"
		end
		if target[-1] == "2"
			xml_uri = "http://cgi2.nhk.or.jp/gogaku/st/xml/#{target[0..-2]}/kouza2/listdataflv.xml"
		end
		if target =~ /^omotenashi_(.*)/
			xml_uri = "http://cgi2.nhk.or.jp/gogaku/st/xml/#{$~[1]}/omotenashi/listdataflv.xml"
		end
	else
		next
	end
	
	print( "#{target}: " )
	open( xml_uri ) { |f|
		doc = REXML::Document.new( f )
		doc.elements.each( 'musicdata/music' ) { |element|
			kouza = element.attributes['kouza']
			hdate = element.attributes['hdate']
			file = element.attributes['file']
			nendo = element.attributes['nendo']
			capture_stream( target, kouza, hdate, file, nendo )
		}
	}
	puts()
}
