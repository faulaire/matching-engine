//-------------------------------------------------------------------------------------------------
// *** f8c generated file: DO NOT EDIT! Created: 2016-04-25 00:14:27 ***
//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-16 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************************
*                Special note for Fix8 compiler generated source code                     *
*                                                                                         *
* Binary works  that are the results of compilation of code that is generated by the Fix8 *
* compiler  can be released  without releasing your  source code as  long as your  binary *
* links dynamically  against an  unmodified version of the Fix8 library.  You are however *
* required to leave the copyright text in the generated code.                             *
*                                                                                         *
*******************************************************************************************

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/

//-------------------------------------------------------------------------------------------------
#include <fix8/f8config.h>
#if defined FIX8_MAGIC_NUM && FIX8_MAGIC_NUM > 16789508L
#error Gateway_traits.cpp version 1.3.4 is out of date. Please regenerate with f8c.
#endif
//-------------------------------------------------------------------------------------------------
// Gateway_traits.cpp
//-------------------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>
#include <cerrno>
#include <string.h>
// f8 includes
#include <fix8/f8exception.hpp>
#include <fix8/hypersleep.hpp>
#include <fix8/mpmc.hpp>
#include <fix8/thread.hpp>
#include <fix8/f8types.hpp>
#include <fix8/f8utils.hpp>
#include <fix8/tickval.hpp>
#include <fix8/logger.hpp>
#include <fix8/traits.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
#include "Gateway_types.hpp"
#include "Gateway_router.hpp"
#include "Gateway_classes.hpp"
//-------------------------------------------------------------------------------------------------
namespace FIX8 {
namespace GateWay {

//-------------------------------------------------------------------------------------------------
// Common group traits
namespace {
} // namespace

//-------------------------------------------------------------------------------------------------
// Message traits
//-------------------------------------------------------------------------------------------------
const FieldTrait Heartbeat::_traits[]
{
   { 112,15,  1,  0,0x04}
};
const FieldTrait_Hash_Array Heartbeat::_ftha(Heartbeat::_traits, Heartbeat::_fieldcnt);
const MsgType Heartbeat::_msgtype("0");
//-------------------------------------------------------------------------------------------------
const FieldTrait TestRequest::_traits[]
{
   { 112,15,  1,  0,0x05}
};
const FieldTrait_Hash_Array TestRequest::_ftha(TestRequest::_traits, TestRequest::_fieldcnt);
const MsgType TestRequest::_msgtype("1");
//-------------------------------------------------------------------------------------------------
const FieldTrait ResendRequest::_traits[]
{
   {   7, 1,  1,  0,0x05}, {  16, 1,  2,  0,0x05}
};
const FieldTrait_Hash_Array ResendRequest::_ftha(ResendRequest::_traits, ResendRequest::_fieldcnt);
const MsgType ResendRequest::_msgtype("2");
//-------------------------------------------------------------------------------------------------
const FieldTrait Reject::_traits[]
{
   {  45, 1,  1,  0,0x05}, {  58,15,  5,  0,0x04}, { 354, 2,  6,  0,0x04}, { 355,28,  7,  0,0x04},
   { 371, 1,  2,  0,0x04}, { 372,15,  3,  0,0x04}, { 373, 1,  4,  0,0x04}
};
const FieldTrait_Hash_Array Reject::_ftha(Reject::_traits, Reject::_fieldcnt);
const MsgType Reject::_msgtype("3");
//-------------------------------------------------------------------------------------------------
const FieldTrait SequenceReset::_traits[]
{
   {  36, 1,  2,  0,0x05}, { 123, 8,  1,  0,0x04}
};
const FieldTrait_Hash_Array SequenceReset::_ftha(SequenceReset::_traits, SequenceReset::_fieldcnt);
const MsgType SequenceReset::_msgtype("4");
//-------------------------------------------------------------------------------------------------
const FieldTrait Logout::_traits[]
{
   {  58,15,  1,  0,0x04}, { 354, 2,  2,  0,0x04}, { 355,28,  3,  0,0x04}
};
const FieldTrait_Hash_Array Logout::_ftha(Logout::_traits, Logout::_fieldcnt);
const MsgType Logout::_msgtype("5");
//-------------------------------------------------------------------------------------------------
const FieldTrait ExecutionReport::_traits[]
{
   {   1,15, 15,  0,0x04}, {   6,11, 50,  0,0x05}, {  11,15,  3,  0,0x04}, {  14,10, 49,  0,0x05},
   {  15,19, 36,  0,0x04}, {  17,15,  8,  0,0x05}, {  18,17, 42,  0,0x04}, {  19,15, 10,  0,0x04},
   {  20, 7,  9,  0,0x05}, {  21, 7, 58,  0,0x04}, {  22,15, 20,  0,0x04}, {  29, 7, 47,  0,0x04},
   {  30,20, 45,  0,0x04}, {  31,11, 44,  0,0x04}, {  32,10, 43,  0,0x04}, {  37,15,  1,  0,0x05},
   {  38,10, 31,  0,0x04}, {  39, 7, 12,  0,0x05}, {  40, 7, 33,  0,0x04}, {  41,15,  4,  0,0x04},
   {  44,11, 34,  0,0x04}, {  48,15, 19,  0,0x04}, {  54, 7, 30,  0,0x05}, {  55,15, 18,  0,0x05},
   {  58,15, 63,  0,0x04}, {  59, 7, 38,  0,0x04}, {  60,22, 55,  0,0x04}, {  63, 7, 16,  0,0x04},
   {  64,25, 17,  0,0x04}, {  66,15,  7,  0,0x04}, {  75,25, 54,  0,0x04}, {  76,15,  6,  0,0x04},
   {  77, 7, 61,  0,0x04}, {  99,11, 35,  0,0x04}, { 103, 1, 13,  0,0x04}, { 106,15, 28,  0,0x04},
   { 107,15, 29,  0,0x04}, { 109,15,  5,  0,0x04}, { 110,10, 59,  0,0x04}, { 111,10, 60,  0,0x04},
   { 113, 8, 64,  0,0x04}, { 119,13, 56,  0,0x04}, { 120,19, 57,  0,0x04}, { 126,22, 41,  0,0x04},
   { 150, 7, 11,  0,0x05}, { 151,10, 48,  0,0x05}, { 152,10, 32,  0,0x04}, { 167,15, 21,  0,0x04},
   { 168,22, 39,  0,0x04}, { 198,15,  2,  0,0x04}, { 200,21, 22,  0,0x04}, { 201, 1, 24,  0,0x04},
   { 202,11, 25,  0,0x04}, { 205, 6, 23,  0,0x04}, { 206, 7, 26,  0,0x04}, { 207,20, 27,  0,0x04},
   { 210,10, 62,  0,0x04}, { 336,15, 46,  0,0x04}, { 376,15, 37,  0,0x04}, { 378, 1, 14,  0,0x04},
   { 424,10, 51,  0,0x04}, { 425,10, 52,  0,0x04}, { 426,11, 53,  0,0x04}, { 432,25, 40,  0,0x04}
};
const FieldTrait_Hash_Array ExecutionReport::_ftha(ExecutionReport::_traits, ExecutionReport::_fieldcnt);
const MsgType ExecutionReport::_msgtype("8");
//-------------------------------------------------------------------------------------------------
const FieldTrait OrderCancelReject::_traits[]
{
   {   1,15,  9,  0,0x04}, {  11,15,  3,  0,0x05}, {  37,15,  1,  0,0x05}, {  39, 7,  5,  0,0x05},
   {  41,15,  4,  0,0x05}, {  58,15, 13,  0,0x04}, {  60,22, 10,  0,0x04}, {  66,15,  8,  0,0x04},
   {  76,15,  7,  0,0x04}, { 102, 1, 12,  0,0x04}, { 109,15,  6,  0,0x04}, { 198,15,  2,  0,0x04},
   { 354, 2, 14,  0,0x04}, { 355,28, 15,  0,0x04}, { 434, 7, 11,  0,0x05}
};
const FieldTrait_Hash_Array OrderCancelReject::_ftha(OrderCancelReject::_traits, OrderCancelReject::_fieldcnt);
const MsgType OrderCancelReject::_msgtype("9");
//-------------------------------------------------------------------------------------------------
const FieldTrait Logon::_traits[]
{
   {  95, 2,  3,  0,0x04}, {  96,28,  4,  0,0x04}, {  98, 1,  1,  0,0x05}, { 108, 1,  2,  0,0x05},
   { 141, 8,  5,  0,0x04}, { 383, 1,  6,  0,0x04}, { 384, 1,  7,  0,0x0c}
};
const FieldTrait_Hash_Array Logon::_ftha(Logon::_traits, Logon::_fieldcnt);
const MsgType Logon::_msgtype("A");
//-------------------------------------------------------------------------------------------------
const FieldTrait Logon::NoMsgTypes::_traits[]
{
   { 372,15,  1,  0,0x04}, { 385, 7,  2,  0,0x04}
};
const FieldTrait_Hash_Array 
   Logon::NoMsgTypes::_ftha(Logon::NoMsgTypes::_traits, NoMsgTypes::_fieldcnt);
const MsgType Logon::NoMsgTypes::_msgtype("NoMsgTypes");
//-------------------------------------------------------------------------------------------------
const FieldTrait NewOrderSingle::_traits[]
{
   {   1,15,  4,  0,0x04}, {  11,15,  1,  0,0x05}, {  12,13, 44,  0,0x04}, {  15,19, 38,  0,0x04},
   {  18,17,  8,  0,0x04}, {  21, 7,  7,  0,0x05}, {  22,15, 16,  0,0x04}, {  38,10, 33,  0,0x04},
   {  40, 7, 35,  0,0x05}, {  44,11, 36,  0,0x04}, {  48,15, 15,  0,0x04}, {  54, 7, 30,  0,0x05},
   {  55,15, 13,  0,0x05}, {  58,15, 46,  0,0x04}, {  59, 7, 40,  0,0x04}, {  60,22, 32,  0,0x05},
   {  63, 7,  5,  0,0x04}, {  64,25,  6,  0,0x04}, {  65,15, 14,  0,0x04}, {  76,15,  3,  0,0x04},
   {  81, 7, 12,  0,0x04}, {  99,11, 37,  0,0x04}, { 100,20, 11,  0,0x04}, { 106,15, 25,  0,0x04},
   { 107,15, 28,  0,0x04}, { 109,15,  2,  0,0x04}, { 110,10,  9,  0,0x04}, { 111,10, 10,  0,0x04},
   { 114, 8, 31,  0,0x04}, { 117,15, 39,  0,0x04}, { 120,19, 45,  0,0x04}, { 126,22, 43,  0,0x04},
   { 140,11, 29,  0,0x04}, { 152,10, 34,  0,0x04}, { 167,15, 17,  0,0x04}, { 168,22, 41,  0,0x04},
   { 200,21, 18,  0,0x04}, { 201, 1, 20,  0,0x04}, { 202,11, 21,  0,0x04}, { 204, 1, 47,  0,0x04},
   { 205, 6, 19,  0,0x04}, { 206, 7, 22,  0,0x04}, { 207,20, 24,  0,0x04}, { 231, 9, 23,  0,0x04},
   { 348, 2, 26,  0,0x04}, { 349,28, 27,  0,0x04}, { 432,25, 42,  0,0x04}
};
const FieldTrait_Hash_Array NewOrderSingle::_ftha(NewOrderSingle::_traits, NewOrderSingle::_fieldcnt);
const MsgType NewOrderSingle::_msgtype("D");
//-------------------------------------------------------------------------------------------------
const FieldTrait OrderCancelRequest::_traits[]
{
   {   1,15,  5,  0,0x04}, {  11,15,  3,  0,0x05}, {  22,15, 11,  0,0x04}, {  37,15,  2,  0,0x04},
   {  38,10, 29,  0,0x04}, {  41,15,  1,  0,0x05}, {  48,15, 10,  0,0x04}, {  54, 7, 27,  0,0x05},
   {  55,15,  8,  0,0x05}, {  58,15, 33,  0,0x04}, {  60,22, 28,  0,0x05}, {  65,15,  9,  0,0x04},
   {  66,15,  4,  0,0x04}, {  76,15,  7,  0,0x04}, { 106,15, 21,  0,0x04}, { 107,15, 24,  0,0x04},
   { 109,15,  6,  0,0x04}, { 152,10, 30,  0,0x04}, { 167,15, 12,  0,0x04}, { 200,21, 13,  0,0x04},
   { 201, 1, 15,  0,0x04}, { 202,11, 16,  0,0x04}, { 205, 6, 14,  0,0x04}, { 206, 7, 17,  0,0x04},
   { 207,20, 20,  0,0x04}, { 223, 9, 19,  0,0x04}, { 231, 9, 18,  0,0x04}, { 348, 2, 22,  0,0x04},
   { 349,28, 23,  0,0x04}, { 350, 2, 25,  0,0x04}, { 351,28, 26,  0,0x04}, { 354, 2, 34,  0,0x04},
   { 355,28, 35,  0,0x04}, { 376,15, 31,  0,0x04}, { 377, 8, 32,  0,0x04}
};
const FieldTrait_Hash_Array OrderCancelRequest::_ftha(OrderCancelRequest::_traits, OrderCancelRequest::_fieldcnt);
const MsgType OrderCancelRequest::_msgtype("F");
//-------------------------------------------------------------------------------------------------
const FieldTrait OrderCancelReplaceRequest::_traits[]
{
   {   1,15,  7,  0,0x04}, {  11,15,  5,  0,0x05}, {  12,13, 54,  0,0x04}, {  13, 7, 55,  0,0x04},
   {  15,19, 48,  0,0x04}, {  18,17, 12,  0,0x04}, {  21, 7, 11,  0,0x05}, {  22,15, 20,  0,0x04},
   {  37,15,  1,  0,0x04}, {  38,10, 38,  0,0x04}, {  40, 7, 40,  0,0x05}, {  41,15,  4,  0,0x05},
   {  44,11, 41,  0,0x04}, {  47, 7, 56,  0,0x04}, {  48,15, 19,  0,0x04}, {  54, 7, 36,  0,0x05},
   {  55,15, 17,  0,0x05}, {  58,15, 59,  0,0x04}, {  59, 7, 49,  0,0x04}, {  60,22, 37,  0,0x05},
   {  63, 7,  9,  0,0x04}, {  64,25, 10,  0,0x04}, {  65,15, 18,  0,0x04}, {  66,15,  6,  0,0x04},
   {  76,15,  3,  0,0x04}, {  77, 7, 64,  0,0x04}, {  78, 1,  8,  0,0x0c}, {  99,11, 42,  0,0x04},
   { 100,20, 15,  0,0x04}, { 106,15, 30,  0,0x04}, { 107,15, 33,  0,0x04}, { 109,15,  2,  0,0x04},
   { 110,10, 13,  0,0x04}, { 111,10, 14,  0,0x04}, { 114, 8, 68,  0,0x04}, { 120,19, 58,  0,0x04},
   { 121, 8, 57,  0,0x04}, { 126,22, 52,  0,0x04}, { 152,10, 39,  0,0x04}, { 167,15, 21,  0,0x04},
   { 168,22, 50,  0,0x04}, { 192,10, 63,  0,0x04}, { 193,25, 62,  0,0x04}, { 200,21, 22,  0,0x04},
   { 201, 1, 24,  0,0x04}, { 202,11, 25,  0,0x04}, { 203, 1, 65,  0,0x04}, { 204, 1, 66,  0,0x04},
   { 205, 6, 23,  0,0x04}, { 206, 7, 26,  0,0x04}, { 207,20, 29,  0,0x04}, { 210,10, 67,  0,0x04},
   { 211,12, 43,  0,0x04}, { 223, 9, 28,  0,0x04}, { 231, 9, 27,  0,0x04}, { 348, 2, 31,  0,0x04},
   { 349,28, 32,  0,0x04}, { 350, 2, 34,  0,0x04}, { 351,28, 35,  0,0x04}, { 354, 2, 60,  0,0x04},
   { 355,28, 61,  0,0x04}, { 376,15, 46,  0,0x04}, { 377, 8, 47,  0,0x04}, { 386, 1, 16,  0,0x0c},
   { 388, 7, 44,  0,0x04}, { 389,12, 45,  0,0x04}, { 427, 1, 53,  0,0x04}, { 432,25, 51,  0,0x04},
   { 439,15, 69,  0,0x04}, { 440,15, 70,  0,0x04}
};
const FieldTrait_Hash_Array OrderCancelReplaceRequest::_ftha(OrderCancelReplaceRequest::_traits, OrderCancelReplaceRequest::_fieldcnt);
const MsgType OrderCancelReplaceRequest::_msgtype("G");
//-------------------------------------------------------------------------------------------------
const FieldTrait OrderCancelReplaceRequest::NoAllocs::_traits[]
{
   {  79,15,  1,  0,0x04}, {  80,10,  2,  0,0x04}
};
const FieldTrait_Hash_Array 
   OrderCancelReplaceRequest::NoAllocs::_ftha(OrderCancelReplaceRequest::NoAllocs::_traits, NoAllocs::_fieldcnt);
const MsgType OrderCancelReplaceRequest::NoAllocs::_msgtype("NoAllocs");
//-------------------------------------------------------------------------------------------------
const FieldTrait OrderCancelReplaceRequest::NoTradingSessions::_traits[]
{
   { 336,15,  1,  0,0x04}
};
const FieldTrait_Hash_Array 
   OrderCancelReplaceRequest::NoTradingSessions::_ftha(OrderCancelReplaceRequest::NoTradingSessions::_traits, NoTradingSessions::_fieldcnt);
const MsgType OrderCancelReplaceRequest::NoTradingSessions::_msgtype("NoTradingSessions");
//-------------------------------------------------------------------------------------------------
const FieldTrait OrderStatusRequest::_traits[]
{
   {   1,15,  4,  0,0x04}, {  11,15,  2,  0,0x05}, {  22,15,  9,  0,0x04}, {  37,15,  1,  0,0x04},
   {  48,15,  8,  0,0x04}, {  54, 7, 25,  0,0x05}, {  55,15,  6,  0,0x05}, {  65,15,  7,  0,0x04},
   {  76,15,  5,  0,0x04}, { 106,15, 19,  0,0x04}, { 107,15, 22,  0,0x04}, { 109,15,  3,  0,0x04},
   { 167,15, 10,  0,0x04}, { 200,21, 11,  0,0x04}, { 201, 1, 13,  0,0x04}, { 202,11, 14,  0,0x04},
   { 205, 6, 12,  0,0x04}, { 206, 7, 15,  0,0x04}, { 207,20, 18,  0,0x04}, { 223, 9, 17,  0,0x04},
   { 231, 9, 16,  0,0x04}, { 348, 2, 20,  0,0x04}, { 349,28, 21,  0,0x04}, { 350, 2, 23,  0,0x04},
   { 351,28, 24,  0,0x04}
};
const FieldTrait_Hash_Array OrderStatusRequest::_ftha(OrderStatusRequest::_traits, OrderStatusRequest::_fieldcnt);
const MsgType OrderStatusRequest::_msgtype("H");
//-------------------------------------------------------------------------------------------------
const FieldTrait header::_traits[]
{
   {   8,15,  1,  0,0x64}, {   9, 1,  2,  0,0x64}, {  34, 1, 10,  0,0x05}, {  35,15,  3,  0,0x44},
   {  43, 8, 11,  0,0x04}, {  49,15,  4,  0,0x05}, {  52,22, 13,  0,0x05}, {  56,15,  5,  0,0x05},
   {  90, 2,  8,  0,0x04}, {  91,28,  9,  0,0x04}, {  97, 8, 12,  0,0x04}, { 115,15,  6,  0,0x04},
   { 122,22, 14,  0,0x04}, { 128,15,  7,  0,0x04}
};
const FieldTrait_Hash_Array header::_ftha(header::_traits, header::_fieldcnt);
const MsgType header::_msgtype("header");
//-------------------------------------------------------------------------------------------------
const FieldTrait BusinessMessageReject::_traits[]
{
   {  45, 1,  1,  0,0x04}, {  58,15,  5,  0,0x04}, { 354, 2,  6,  0,0x04}, { 355,28,  7,  0,0x04},
   { 372,15,  2,  0,0x05}, { 379,15,  3,  0,0x04}, { 380, 1,  4,  0,0x05}
};
const FieldTrait_Hash_Array BusinessMessageReject::_ftha(BusinessMessageReject::_traits, BusinessMessageReject::_fieldcnt);
const MsgType BusinessMessageReject::_msgtype("j");
//-------------------------------------------------------------------------------------------------
const FieldTrait trailer::_traits[]
{
   {  10,15,  3,  0,0x64}, {  89,28,  2,  0,0x04}, {  93, 2,  1,  0,0x04}
};
const FieldTrait_Hash_Array trailer::_ftha(trailer::_traits, trailer::_fieldcnt);
const MsgType trailer::_msgtype("trailer");

} // namespace GateWay
} // namespace FIX8
