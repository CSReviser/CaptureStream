#ifndef UTILITY_H
#define UTILITY_H

#include <QString>

namespace Utility {
	QString applicationBundlePath();
	QString flare( QString& error );
	QString gnash( QString& error );
	QString wiki();
}

#endif // UTILITY_H
