#!/usr/bin/env ruby -Ku
$KCODE = 'UTF8'

require 'open-uri'
require 'rexml/document'
require 'kconv'
require 'nkf'
require 'date'
require 'jcode'
require "tempfile"
require 'fileutils'

=begin

＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝【注意事項】＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
このスクリプトの使用結果については作者は責任を負うことはできません。できる限りのテストを行い、
善意を持って作成しておりますが、すべて使用される方の自己責任でお願いいたします。

＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝【更新履歴】＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
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
2010/05/12　ゴールデンウィーク対応。flvファイル名に追加される"mm"と"vip"に対応。
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
このスクリプトを実行するためにはRubyとflvstreamerが必要です。flvstreamerはパスの通ったフォ
ルダに存在する必要があります。

以下のURLまたは適当な場所からダウンロードするか、ご自分でビルド／インストールしてください。
以下のURLはあくまで参考のために載せているだけで、リンク先のバイナリの内容については責任を持
てません。ご自身で判断の上、お使いください。

【Windows環境】
・Ruby
	下記のRubyInstaller for Windowsのサイトから1.8.7の最新版をインストールしてください。
	http://rubyinstaller.org/downloads/
	このドキュメントを記述している時の最新版のリンクです（動作確認したバージョンです）。
	http://rubyforge.org/frs/download.php/74293/rubyinstaller-1.8.7-p334.exe
・flvstreamer
	http://ftp.twaren.net/Unix/NonGNU/flvstreamer/win32/flvstreamer_win32_latest.exe
	上記のファイルをブラウザでダウンロードした後、ファイル名をflvstreamer.exeに変更し、
	パスの通っているフォルダに置いてください。Windowsの場合は、CaptureStream.rbと同じフォ
	ルダでもOKです。

【Macintosh環境】
・Ruby
	標準でインストールされています。
・flvstreamer
	http://ftp.twaren.net/Unix/NonGNU/flvstreamer/macosx/flvstreamer_macosx_unified_binary_latest
	上記のファイルをブラウザでダウンロードした後、ファイル名をflvstreamerに変更し、ターミ
	ナルで実行属性を付け、パスの通ったディレクトリに置いてください。

【Linux環境】
・Ruby
	パッケージでインストール可能だと思います。
・flvstreamer
	http://ftp.twaren.net/Unix/NonGNU/flvstreamer/linux/
	上記の場所からご自分のシステムに最適なものをダウンロードし、パスの通ったディレクトリに
	置いてください。
	もしくは、パッケージマネージャを使用してインストールしてください。
	
＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝【使用方法]＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
・Windowsの場合
	DOSプロンプトを開き、このスクリプトの入っているフォルダに移動して「ruby CaptureStream.rb 講座名」
	を実行します。もしくは、下記に従ってデフォルトの動作を設定してあれば、CaptureStream.rb
	をダブルクリックしても実行可能のはずです。

・MacintoshおよびLinuxの場合
	ターミナルを起動し、このスクリプトの入っているフォルダに移動して「ruby CaptureStream.rb 講座名」
	を実行します。

講座名のところには以下のものが複数指定可能です。allを指定するとすべての講座をダウンロードします。


basic1 basic2 basic3 timetrial kaiwa business1 business2 chinese french italian 
hangeul german spanish russian levelup-chinese levelup-hangeul enews shower all

$default_target（配列）に指定しておくことで引数指定なしでダウンロードさせることができます。
入門ビジネス英語と実践ビジネス英語を指定するには以下のように設定します。

	$default_target = ["business1", "business2"]

=end

#--------------------------------------------------------------------------------
# 講座名リスト
#--------------------------------------------------------------------------------

$default_target = []
$english = ["basic1", "basic2", "basic3", "timetrial", "kaiwa", "business1", "business2", "kouryaku"]
$multilingual = ["chinese", "french", "italian", "hangeul", "german", "spanish", "russian", "levelup-chinese", "levelup-hangeul"]
$extra = ["enews", "shower", "enews-all"]

#--------------------------------------------------------------------------------
# 実行環境の検出とツールのパス設定
#--------------------------------------------------------------------------------

$is_windows = RUBY_PLATFORM.downcase =~ /mswin(?!ce)|mingw|cygwin|bccwin/
$script_path = File.expand_path( __FILE__ )
# -m 0はv1.5からのオプションでタイムアウトしない設定
$flvstreamer = "flvstreamer -m 0"
$ffmpeg = "ffmpeg"

if $is_windows
	$script_path = $script_path.kconv( Kconv::UTF8, Kconv::SJIS )
	$null = "nul"
else
	$null = "/dev/null"
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
# 2011年度のストリーミングのURLに追加されたダウンロード妨害用文字列対応
# 優先順位は、マニュアル設定→ウィキから日付指定で取得→gnashでの自動検出
# 何らかの問題でウィキからスクランブル文字列が取得できない場合には自分で設定してください
#--------------------------------------------------------------------------------

jputs( "語学講座ダウンローダ (2013/04/08)" )

$scramble = ""

# ウィキから日付を指定してスクランブル文字列を取得する
if $scramble == ""
	now = DateTime.now
	offset = 1 - now.cwday	#直前の月曜までのオフセット
	if offset == 0 && now.hour <= 9	#月曜日で10時より前なら1週間前の月曜日に
		offset = -7
	end
	monday = Date.today + offset
	
	xml_uri = "http://cdn47.atwikiimg.com/jakago/pub/scramble.xml"
	open( xml_uri ) { |f|
		doc = REXML::Document.new( f )
		$scramble = doc.elements["flv/scramble[@date=\"#{monday.strftime( '%Y%m%d' )}\"]/@code"].to_s
	}
	if $scramble != ""
		jputs( "wikiから取得したコード：#$scramble" )
	else
		jputs( "wikiから取得したコード： 取得に失敗したか、まだwikiのxmlが更新されていません。" )
	end
end

if $scramble == ""
	jputs( "スクランブル文字列が取得できません。" )
	exit
end

#--------------------------------------------------------------------------------
# flvファイルのサーバとオプション
#--------------------------------------------------------------------------------

$flv_host = "flv.nhk.or.jp"
$flv_app = "ondemand/"
$flv_service_prefix = "mp4:flv/gogaku/streaming/mp4/#{$scramble}/"

#--------------------------------------------------------------------------------
# 出力フォルダ名とmp3ファイル名の設定
# 以下は基礎英語1を例にして書いてありますが、他の講座は適宜読み替えてください。
# %H %K %F は以下のURLにあるXMLに含まれる属性の指定方法です。
# http://www.nhk.or.jp/gogaku/english/basic1/listdataflv.xml
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
# フォルダ名の例： "%r%p英語%y%M" -> （このスクリプトがあるフォルダ内の）英語0905
# ファイル名の例： "DE%Y%M%D.mp3" -> DE20090420.mp3
#--------------------------------------------------------------------------------

# 保存フォルダ名
$out_folder_hash = {"basic1"=>"%r%p%k", "basic2"=>"%r%p%k", "basic3"=>"%r%p%k", "timetrial"=>"%r%p%k", "kaiwa"=>"%r%p%k", "business1"=>"%r%p%k", "business2"=>"%r%p%k", "kouryaku"=>"%r%p%k", "chinese"=>"%r%p%k", "french"=>"%r%p%k", "italian"=>"%r%p%k", "hangeul"=>"%r%p%k", "german"=>"%r%p%k", "spanish"=>"%r%p%k", "russian"=>"%r%p%k", "levelup-chinese"=>"%r%p%k", "levelup-hangeul"=>"%r%p%k", "enews"=>"%r%p%k", "shower"=>"%r%p%k"
}

# 保存ファイル名
$out_file_hash = {"basic1"=>"%k_%Y_%M_%D.mp3", "basic2"=>"%k_%Y_%M_%D.mp3", "basic3"=>"%k_%Y_%M_%D.mp3", "timetrial"=>"%k_%Y_%M_%D.mp3", "kaiwa"=>"%k_%Y_%M_%D.mp3", "business1"=>"%k_%Y_%M_%D.mp3", "business2"=>"%k_%Y_%M_%D.mp3", "kouryaku"=>"%k_%Y_%M_%D.mp3", "chinese"=>"%k_%Y_%M_%D.mp3", "french"=>"%k_%Y_%M_%D.mp3", "italian"=>"%k_%Y_%M_%D.mp3", "hangeul"=>"%k_%Y_%M_%D.mp3", "german"=>"%k_%Y_%M_%D.mp3", "spanish"=>"%k_%Y_%M_%D.mp3", "russian"=>"%k_%Y_%M_%D.mp3", "levelup-chinese"=>"%k_%Y_%M_%D.mp3", "levelup-hangeul"=>"%k_%Y_%M_%D.mp3", "enews"=>"%k_%Y_%M_%D.mp3", "shower"=>"%k_%Y_%M_%D.mp3"
}

# id3タグのalbum
$id3_album = {"basic1"=>"%k", "basic2"=>"%k", "basic3"=>"%k", "timetrial"=>"%k", "kaiwa"=>"%k", "business1"=>"%k", "business2"=>"%k", "kouryaku"=>"%k", "chinese"=>"%k", "french"=>"%k", "italian"=>"%k", "hangeul"=>"%k", "german"=>"%k", "spanish"=>"%k", "russian"=>"%k", "levelup-chinese"=>"%k", "levelup-hangeul"=>"%k", "enews"=>"%k", "shower"=>"%k"
}

# id3タグのtitle
$id3_title = {"basic1"=>"%k_%Y_%M_%D", "basic2"=>"%k_%Y_%M_%D", "basic3"=>"%k_%Y_%M_%D", "timetrial"=>"%k_%Y_%M_%D", "kaiwa"=>"%k_%Y_%M_%D", "business1"=>"%k_%Y_%M_%D", "business2"=>"%k_%Y_%M_%D", "kouryaku"=>"%k_%Y_%M_%D", "chinese"=>"%k_%Y_%M_%D", "french"=>"%k_%Y_%M_%D", "italian"=>"%k_%Y_%M_%D", "hangeul"=>"%k_%Y_%M_%D", "german"=>"%k_%Y_%M_%D", "spanish"=>"%k_%Y_%M_%D", "russian"=>"%k_%Y_%M_%D", "levelup-chinese"=>"%k_%Y_%M_%D", "levelup-hangeul"=>"%k_%Y_%M_%D", "enews"=>"%k_%Y_%M_%D", "shower"=>"%k_%Y_%M_%D"
}

#--------------------------------------------------------------------------------
# 出力ファルが存在する場合にダウンロードをスキップする場合はtrueを、しない場合はfalseを設定
#--------------------------------------------------------------------------------

$skip_existing = true

#--------------------------------------------------------------------------------
# 音声ファイルの変換後の拡張子を設定（flvを指定するとダウンロードしたファイルそのもの）
# 3g2, 3gp, aac, avi, flv, m2ts, m4a, mka, mkv, mov, mp3, mp4
#--------------------------------------------------------------------------------

$audio_extension = "aac"

#--------------------------------------------------------------------------------
# 出力フォルダ名とmp3ファイル名のフォーマット文字列の解釈
#--------------------------------------------------------------------------------

# 引数はxmlのhdateとfileアトリビュート
def make_date( hdate, file )
	hdate =~ /^(\d+)\D+(\d+)/
	raise "XMLに含まれている放送日の形式が違います。" if !$1 || $1.length < 1 || $1.length > 2 || !$2 || $2.length < 1 || $2.length > 2
	month = $1
	day = $2
	
	year = 2000 + file[0,2].to_i
	year += 1 if month =~ /^[123]$/ && Date.today.year > year
	return Date.new( year, month.to_i, day.to_i )
end

def format_name( format, target, kouza, hdate, file )
	result = String.new()
	
	hdate =~ /^(\d+)\D+(\d+)/
	raise "XMLに含まれている放送日の形式が違います。" if !$1 || $1.length < 1 || $1.length > 2 || !$2 || $2.length < 1 || $2.length > 2
	month = $1
	day = $2
	
	if target == "enews"
		year = 2000 + file[2,2].to_i
	elsif target == "shower"
		year = 2000 + file[3,2].to_i
	else
		year = 2000 + file[0,2].to_i
	end
	year += 1 if month =~ /^[123]$/ && Date.today.year > year
	file = File.basename( file )	# ニュースで英会話が"201004/23_Fri/el/video/20100423_PULITZER"の形式のため
	
	chars = format.each_char
	percent = false
	i = 0
	while i < chars.length
		if percent
			percent = false
			case chars[i]
			when "k"
				result << kouza
			when "h"
				result << hdate
			when "f"
				if file =~ /(.*)\.flv$/
					result << $1
				else
					result << file
				end
			when "r"
				result << File.dirname( $script_path )
			when "p"
				result << File::Separator
			when "Y"
				result << year.to_s
			when "y"
				result << (year % 100).to_s
			when "M"
				result << "0" if month.length < 2
				result << month
			when "m"
				result << month
			when "D"
				result << "0" if day.length < 2
				result << day
			when "d"
				result = day
			else
				result << chars[i]
			end
		else
			if chars[i] == "%"
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
# タグの書き込み
#--------------------------------------------------------------------------------

def encode_size( int32 )
	result = String.new()
	result << ((int32 & (0x0000007f << 21)) >> 21).chr
	result << ((int32 & (0x0000007f << 14)) >> 14).chr
	result << ((int32 & (0x0000007f << 7)) >> 7).chr
	result << (int32 & 0x0000007f).chr
	return result
end

# identifier and string must be UTF-8
def ascii_frame( identifier, string )
	ascii_mark = "\x00"
	result = String.new()

	if identifier.size == 3 && string.size > 0
		frame_data = NKF.nkf( "-l -W", string )
		if frame_data.size > 0
			frame_data = ascii_mark + frame_data + "\x00"
			result << identifier
			result << ((frame_data.size & 0x00ff0000) >> 16).chr
			result << ((frame_data.size & 0x0000ff00) >> 8).chr
			result << (frame_data.size & 0x000000ff).chr
			result << frame_data
		end
	end

	return result
end

# identifier and string must be UTF-8
def unicode_frame( identifier, string )
	unicode_mark = "\x01"
	result = String.new()

	if identifier.size == 3 && string.size > 0
		frame_data = NKF.nkf( "-w16L -W", string )
		if frame_data.size > 0
			frame_data = unicode_mark + frame_data + "\x00\x00"
			result << identifier
			result << ((frame_data.size & 0x00ff0000) >> 16).chr
			result << ((frame_data.size & 0x0000ff00) >> 8).chr
			result << (frame_data.size & 0x000000ff).chr
			result << frame_data
		end
	end

	return result
end

def create_tag( album, title, year, artist )
	tag_bytes = String.new()
	frames = String.new()

	frames << unicode_frame( "TAL", album );
	frames << unicode_frame( "TT2", title );
	frames << ascii_frame( "TYE", year );
	frames << unicode_frame( "TP1", artist );
	frames << ascii_frame( "TCO", "(101)" );

	if frames.size > 0
		tag_bytes << "ID3\x02\x00\x00"
		tag_bytes << encode_size( frames.size )
		tag_bytes << frames
	end

	return tag_bytes
end

def decode_size( byte4 )
	return ((byte4[0] & 0x7f) << 21) + ((byte4[1] & 0x7f) << 14) +
			((byte4[2] & 0x7f) << 7) + (byte4[3] & 0x7f)
end

# 2.2.0~2.4.0のタグのサイズを計算する
def tag_size( buffer )
	#offset_identifier = 0
	offset_version = 3
	#offset_flags = 5
	offset_size = 6
	offset_data = 10
	identifier = "ID3"

	result = 0

	if buffer.size > offset_data && buffer[0,3] == identifier
		# ID3v2.2.0 と ID3v2.3.0、ID3v2.4.0 のみサポート
		if buffer[offset_version] >= "2"[0] && buffer[offset_version] <= "4"[0] && buffer[offset_version + 1] == "0"[0]
			result = decode_size( buffer[offset_size,4] ) + offset_data
		end
	end

	return result;
end

def make_temp_name( original )
	temp_file = Tempfile.open( File.basename( original ) )
	result = temp_file.path
	temp_file.close!
	return result
end

def id3tag( full_path, album, title, year )	# full_pathはto_nativeで変換されている
	src_file = nil
	dst_file = nil
	original_file_moved = false

	begin
		tag_bytes = create_tag( album, title, year, "NHK" )
		raise  "書き込むべきタグが見当たらないため、タグの書き込みを中止します。" if tag_bytes.size <= 0
		temp_name = make_temp_name( full_path )
		raise "作業用ファイル名が作成できないため、タグの書き込みを中止します。" if temp_name.size <= 0
		FileUtils.move( full_path, temp_name )
		original_file_moved = true
		src_file = File.open( temp_name, "r" )
		src_file.binmode
		dst_file = File.open( full_path, "w" )
		dst_file.binmode
		raise "作業用ファイルへの書き込みに失敗しました。" if dst_file.write( tag_bytes ) != tag_bytes.size
		buffer = src_file.read
		skip = tag_size( buffer )
		raise "作業用ファイルへの書き込みに失敗しました。" if dst_file.write( buffer[skip..-1] ) != buffer.size - skip
		dst_file.close
		dst_file = nil
		src_file.close
		File.unlink( temp_name )
		src_file = nil
	rescue
		jputs( $! )
		( src_file.close if src_file ) rescue false
		( dst_file.close if dst_file ) rescue false
		( File.unlink( full_path ) if dst_file ) rescue false
		( FileUtils.move( temp_name, full_path ) if original_file_moved ) rescue false
	end
end

#--------------------------------------------------------------------------------
# flvからmp3への変換
#--------------------------------------------------------------------------------

FLV_HEADER_SIZE = 9
FLV_TAG_SIZE = 15
FLV_HEADER = Struct.new( :signature, :version, :flags, :offset )
FLV_TAG = Struct.new( :previousTagSize, :type, :bodyLength, :timestamp, :timestampExtended, :streamId )

def flv2mp3( flv_path, mp3_path )	# flv_pathとmp3_pathはto_nativeで変換されている
	result = false

	begin
		# Windowsのためにバイナリモードを指定しなければならない
		port = open( flv_path, "rb" )
		begin
			flv = port.read
		ensure
			port.close
		end
		header = FLV_HEADER.new( flv[0,3], flv[3], flv[4], flv[5, 4] )
		
		raise "flvファイルにヘッダが含まれていません。" if flv.length < FLV_HEADER_SIZE
		raise "flvファイルではありません。" unless header.signature == "FLV"
		raise "音声データが含まれていません。" if (header.flags & 4) == 0
		raise "flvファイルが対応できる形式ではありません。" unless header.offset == "\x00\x00\x00#{FLV_HEADER_SIZE.chr}"
		
		read_size = FLV_HEADER_SIZE
		
		open( mp3_path, "wb" ) { |mp3|
			while true
				remaining = flv.length - read_size
				break if remaining == 4 # 最後のPreviousTagSize => 完了
				raise "flvファイルの内容が不正です。" if remaining < FLV_TAG_SIZE
				tag = FLV_TAG.new( flv[read_size, 4], flv[read_size + 4], flv[read_size + 5, 3], flv[read_size + 8, 3], flv[read_size + 11], flv[read_size + 12, 3] )
				read_size += FLV_TAG_SIZE
				body_length = (tag.bodyLength[0] << 16) + (tag.bodyLength[1] << 8) + tag.bodyLength[2];
				raise "flvファイルの内容が不正です。" if remaining < body_length
				if tag.type == 8
					raise "音声データがmp3ではありません。" if (flv[read_size] & 0x00f0) != 0x20
					raise "mp3ファイルの書き込みに失敗しました。" if mp3.write( flv[read_size + 1, body_length - 1] ) != body_length - 1
				end
				read_size += body_length
			end
		}
		result = true
	rescue
		puts( to_native( $!.to_s + "：" ) + flv_path )
		File.delete( mp3_path ) if File.exist?( mp3_path )
	end

	return result
end

#--------------------------------------------------------------------------------
# flvのダウンロードとmp3への変換
#--------------------------------------------------------------------------------

def capture_stream( target, kouza, hdate, file, retry_count )
	out_folder = format_name( $out_folder_hash[target], target, kouza, hdate, file )
	out_file = format_name( $out_file_hash[target], target, kouza, hdate, file )
	id3_album = format_name( $id3_album[target], target, kouza, hdate, file )
	id3_title = format_name( $id3_title[target], target, kouza, hdate, file )
	
	#既存ファイル／フォルダのチェックのために、フォルダ名の末尾のセパレータがあれば取り除く
	parts = File.split( out_folder )
	out_folder = parts[0] + File::Separator + parts[1]
	
	exit unless check_output_dir( out_folder )
	out_folder += File::Separator
	
	out_folder = to_native( out_folder )
	out_file = to_native( out_file )
	out_file = File.basename( out_file, File.extname( out_file ) ) + "." + $audio_extension
	
	return true if $skip_existing && File.exists?( "#{out_folder}#{out_file}" )
	
	result = false
	if file =~ /(.*)\.flv$/ || file =~ /(.*)\.mp4$/
		basename = File.basename( file )
		wo_extname = File.basename( basename, File.extname( basename ) )
		command1935 = "#{$flvstreamer} -r \"rtmp://#{$flv_host}/#{$flv_app}#{$flv_service_prefix}#$1\" -o \"#{out_folder}#{wo_extname}.flv\" > #{$null} 2>&1"
		command80 = "#{$flvstreamer} -r \"rtmpt://#{$flv_host}:80/#{$flv_app}#{$flv_service_prefix}#$1\" -o \"#{out_folder}#{wo_extname}.flv\" > #{$null} 2>&1"
		system( command1935 )
		if $?.to_i == 0x7f00
			jputs( "\nflvstreamerが実行できません。パスの通った実行可能な場所にflvstreamerを置いてください。" )
			exit
		end
		while $? != 0 && retry_count > 0
			system( "#{command80} --resume" )
			retry_count -= 1
		end
		
		if $? == 0
			if $audio_extension == "flv"
				File.rename( "#{out_folder}#{wo_extname}.flv", out_folder + out_file )
			else
				if $audio_extension == "mp3"
					system( "#{$ffmpeg} -i \"#{out_folder}#{wo_extname}.flv\" -vn -acodec libmp3lame -ar 22050 -ac 1 -ab 48k -y \"#{out_folder}#{out_file}\" > #{$null} 2>&1" )
				else
					system( "#{$ffmpeg} -i \"#{out_folder}#{wo_extname}.flv\" -vn -acodec copy -y \"#{out_folder}#{out_file}\" > #{$null} 2>&1" )
				end
				if $?.to_i == 0x7f00
					jputs( "\nffmpegが実行できません。パスの通った実行可能な場所にflvstreamerを置いてください。" )
					exit
				end
				if $? == 0
					if $audio_extension == "mp3"
						id3tag( out_folder + out_file, id3_album, id3_title, "20" + file[0..1] )
					end
				end
			end
			result = true
			print( "*" )
		end
=begin
		if $? == 0
			if flv2mp3( out_folder + basename, out_folder + out_file )
				id3tag( out_folder + out_file, id3_album, id3_title, "20" + file[0..1] )
				result = true
				print( "*" )
			end
		end
=end
	end
	
=begin
	if File.exists?( out_folder + basename )
		if !result || ( target != "enews" && target != "shower" )
			File.unlink( out_folder + basename )
		else
			movie_name = File.basename( out_file, File.extname( out_file ) ) + File.extname( basename )
			File.rename( out_folder + basename, out_folder + movie_name )
		end
	end
=end
	
	return result
end

#--------------------------------------------------------------------------------
# ニュースで英会話
#--------------------------------------------------------------------------------

SEARCH_20100323 = 'http://www.google.co.jp/search?q=video_player_wide.swf+site:cgi2.nhk.or.jp&hl=ja&lr=lang_ja&num=100&filter=0&start='
SEARCH_20090330 = 'http://www.google.co.jp/search?q=video_player.swf+site:cgi2.nhk.or.jp&hl=ja&lr=lang_ja&num=100&filter=0&start='
REGEXP = %r|video_player(?:_wide)?\.swf\?type=real&(?:amp;)?m_name=([^"]*)|
ENEWS = 'http://cgi2.nhk.or.jp/e-news/news/index.cgi?ymd='
FLV_SERVICE_PREFIX_20090728 = 'e-news/data/'
FLV_SERVICE_PREFIX_20090330 = 'e-news-flv/'

def get_enews_names( search, regexp )
	result = Array.new
	for j in (0..3)	# 一度では取りこぼしがあるので
		i = 0
		temp = Array.new
		while true
			open( search + i.to_s ) { |file|
				search_result = file.read
				while regexp =~ search_result
					temp << $1
					search_result = $~.post_match
				end
			}
			i += 100
			break if temp.size < i
		end
		result += temp
	end
	return result.uniq
end

# NHKのHPで現在公開中のファイル名を取得
def current_list
	result = Array.new
	today = Date.today
	i = today - 7
	while i <= today
		if i.wday >= 1 && i.wday <= 5	# 月曜から金曜まで
			open( "#{ENEWS}#{i.strftime( '%Y%m%d' )}" ) { |file|
				result << $1 if REGEXP =~ file.read
			}
		end
		i += 1
	end
	return result
end

# NHKのHPで現在公開中のファイルをダウンロード
def download_enews
	print( "enews: " )
	flv_service_prefix = $flv_service_prefix
	$flv_service_prefix = FLV_SERVICE_PREFIX_20090728
	
	current_list.each { |flv|
		capture_stream( "enews", "ニュースで英会話", "#{flv[4,2].to_i}月#{flv[7,2].to_i}日放送分", "#{flv}.flv", 5 )
	}
	
	$flv_service_prefix = flv_service_prefix
	puts()
end

# 公開中のファイルとGoogle検索で見つかった過去分すべてをダウンロード
def download_enews_all
	# Googleでの検索結果から2010/03/23以降のファイル名を取得
	flvs_20100323 = get_enews_names( SEARCH_20100323, REGEXP )
	flvs_20100323 += current_list
	
	# Googleでの検索結果から2010/03/22以前のファイル名を取得
	flvs = get_enews_names( SEARCH_20090330, REGEXP )
	
	# 2010/03/22以前のファイルを2009/07/28で振り分ける
	flvs_20090330 = Array.new
	flvs_20090728 = Array.new
	flvs.each { |flv|
		(flv[0, 6] + flv[7, 2]).to_i >= 20090728 ? (flvs_20090728 << flv) : (flvs_20090330 << flv)
	}
	
	print( "enews-all: " )
	# 2009/07/28以降のflvをダウンロード
	flv_service_prefix = $flv_service_prefix
	$flv_service_prefix = FLV_SERVICE_PREFIX_20090728
	(flvs_20100323 + flvs_20090728).sort.reverse.uniq.each { |flv|	# 同じ日付で更新されたもの(_newが付いている)があるので逆順にソートしておく
		capture_stream( "enews", "ニュースで英会話", "#{flv[4,2].to_i}月#{flv[7,2].to_i}日放送分", "#{flv}.flv", 5 )
	}
	
	# 2009/07/27以前のflvをダウンロード
	$flv_service_prefix = FLV_SERVICE_PREFIX_20090330
	flvs_20090330.sort.reverse.uniq.each { |flv|	# 念のため逆順にソートしておく
		capture_stream( "enews", "ニュースで英会話", "#{flv[4,2].to_i}月#{flv[7,2].to_i}日放送分", "#{flv}.flv", 5 )
	}
	puts()
	$flv_service_prefix = flv_service_prefix
end

#--------------------------------------------------------------------------------
# ABCニュースシャワー
#--------------------------------------------------------------------------------

def download_shower
	flv_service_prefix = $flv_service_prefix
	$flv_service_prefix = 'worldwave/common/movie/'
	
	print( "ABC News Shower: " )
	xml_uri = "http://www.nhk.or.jp/worldwave/xml/abc_news.xml"
	
	open( xml_uri ) { |f|
		doc = REXML::Document.new( f )
		doc.elements.each( "rss/channel/item/pubDate" ) { |element|
			date = Date.parse( element.text )
			kouza = "ABCニュースシャワー"
			hdate = "#{date.month}月#{date.day}日放送分"
			file = "abc" + date.strftime( '%y%m%d' ) + ".flv"
			capture_stream( "shower", kouza, hdate, file, 5 )
		}
	}
	puts()
	
	$flv_service_prefix = flv_service_prefix
end

#--------------------------------------------------------------------------------
# メインプログラム
#--------------------------------------------------------------------------------

Dir.chdir( to_native( File.dirname( $script_path ) ) )
targets = ARGV.length > 0 ? ARGV : $default_target
targets = $english + $multilingual + $extra if targets.include?( "all" )

targets.each { |target|
	if !$english.include?( target ) && !$multilingual.include?( target ) && !$extra.include?( target )
		jputs( "サポートされていない講座名です：#{target}" )
		jputs( "使用方法: ruby #$PROGRAM_NAME [#{$english.join('|')}|#{$multilingual.join('|')}|#{$extra.join('|')}|all]" )
		exit
	end
}

targets.each { |target|
	if $english.include?( target )
		xml_uri = "http://cgi2.nhk.or.jp/gogaku/english/#{target}/#{$scramble}/listdataflv.xml"
	elsif $multilingual.include?( target )
		if target =~ /^levelup-(.*)/
			xml_uri = "http://cgi2.nhk.or.jp/gogaku/#{$~[1]}/levelup/#{$scramble}/listdataflv.xml"
		else
			xml_uri = "http://cgi2.nhk.or.jp/gogaku/#{target}/kouza/#{$scramble}/listdataflv.xml"
		end
	else
		next
	end
	
	print( "#{target}: " )
	open( xml_uri ) { |f|
		doc = REXML::Document.new( f )
		doc.elements.each( "musicdata/music" ) { |element|
			kouza = element.attributes["kouza"]
			hdate = element.attributes["hdate"]
			file = element.attributes["file"]
			capture_stream( target, kouza, hdate, file, 5 )
		}
	}
	puts()
}

download_shower if targets.include?( "shower" )
#download_enews if targets.include?( "enews" )
#download_enews_all if targets.include?( "enews-all" )
jputs( "現在、「ニュースで英会話」はサポート外です。" ) if targets.include?( "enews" ) || targets.include?( "enews-all" )
