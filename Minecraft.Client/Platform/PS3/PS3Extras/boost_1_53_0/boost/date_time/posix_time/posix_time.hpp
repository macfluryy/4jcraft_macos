#ifndef POSIX_TIME_HPP___
#define POSIX_TIME_HPP___

/* Copyright (c) 2002-2005 CrystalClear Software, Inc.
 * Use, modification and distribution is subject to the 
 * Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)
 * Author: Jeff Garland, Bart Garst 
 * $Date: 2008-02-27 12:00:24 -0800 (Wed, 27 Feb 2008) $
 */
/*!@file posix_time.hpp Global header file to get all of posix time types 
 */

#include "../compiler_config.hpp"
#include "ptime.hpp"
#if defined(BOOST_DATE_TIME_OPTIONAL_GREGORIAN_TYPES)
#include "date_duration_operators.hpp"
#endif

// output functions
#if defined(BOOST_DATE_TIME_INCLUDE_LIMITED_HEADERS)
#include "time_formatters_limited.hpp"
#else
#include "time_formatters.hpp"
#endif // BOOST_DATE_TIME_INCLUDE_LIMITED_HEADERS

// streaming operators
#if defined(USE_DATE_TIME_PRE_1_33_FACET_IO)
#include "posix_time_legacy_io.hpp"
#else
#include "posix_time_io.hpp"
#endif // USE_DATE_TIME_PRE_1_33_FACET_IO

#include "time_parsers.hpp"
#include "conversion.hpp"


#endif

