/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_TEMPLATES_HPP_
#define SRC_LIB_DBUS_EVENTS_GKDBUS_EVENT_TEMPLATES_HPP_

// "s" - string
// "a" - array of
// "v" - void
// "b" - bool
// "y" - byte
// "t" - uint64_t
// "m" - M-KeyID
// "G" - G-KeyID
// "M" - Macro
// "B" - Bank (mBank_type)
// "P" - LCD Plugins Properties

#include "SIGas2v.hpp"    //         array of string to void
#include "SIGs2as.hpp"    //       string to array of string
#include "SIGs2b.hpp"     //                  string to bool
#include "SIGs2s.hpp"     //                string to string
#include "SIGs2v.hpp"     //                  string to void
#include "SIGsmG2b.hpp"   // one string one M-KeyID one G-KeyID to bool
#include "SIGss2aG.hpp"   //  two strings to array of G-KeyID
#include "SIGss2aP.hpp"   //  two strings to array of LCD Plugins Properties
#include "SIGss2as.hpp"   //  two strings to array of string
#include "SIGss2b.hpp"    //             two strings to bool
#include "SIGss2s.hpp"    //           two strings to string
#include "SIGss2v.hpp"    //             two strings to void
#include "SIGssm2b.hpp"   // two strings one M-KeyID to bool
#include "SIGssmB2b.hpp"  // two strings one M-KeysID one mBank_type to bool
#include "SIGssmG2M.hpp"  // two strings one M-KeyID one G-KeyID to macro
#include "SIGssyt2b.hpp"  // two strings one byte one uint64_t to bool
#include "SIGssyyy2b.hpp" // two strings three bytes to bool
#include "SIGv2v.hpp"     //                    void to void


/* -- -- -- -- -- -- -- -- -- -- -- -- */
/* -- -- -- classes templates -- -- -- */
/* -- -- -- -- -- -- -- -- -- -- -- -- */


#endif
