#ifndef MATCHER_GLOBAL_H
#define MATCHER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MATCHER_LIBRARY)
#  define MATCHERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MATCHERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MATCHER_GLOBAL_H
