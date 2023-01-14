/*
	Copyright (C) 2009-2013 jakago

	This file is part of CaptureStream, the flv downloader for NHK radio
	language courses.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UTILITY_H
#define UTILITY_H

#include <QString>

namespace Utility {
	QString applicationBundlePath();
	QString appLocaldataLocationPath();
	QString appConfigLocationPath();
	QString ConfigLocationPath();
	QString DownloadLocationPath();	
	QString HomeLocationPath();
	QString flare( QString& error );
	QString gnash( QString& error );
	QString wiki();
	bool nogui();
}

#endif // UTILITY_H
