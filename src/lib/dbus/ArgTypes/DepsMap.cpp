/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.hpp"

#include "DepsMap.hpp"

namespace NSGKDBus
{

using namespace NSGKUtils;

void TypeGKDepsMap::appendGKDepsMap(const GLogiK::GKDepsMap_type & depsMap)
{
	this->appendGKDepsMap(&_itMessage, depsMap);
}

void TypeGKDepsMap::appendGKDepsMap(DBusMessageIter *iter, const GLogiK::GKDepsMap_type & depsMap)
{
	GK_LOG_FUNC

	this->appendUInt64(iter, depsMap.size()); // map size

	DBusMessageIter itArray_1;

	// signature = (yta(sss))
	const char array_1_sig[] = \
		DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
		DBUS_TYPE_BYTE_AS_STRING\
		DBUS_TYPE_UINT64_AS_STRING\
		DBUS_TYPE_ARRAY_AS_STRING\
		DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
		DBUS_TYPE_STRING_AS_STRING\
		DBUS_TYPE_STRING_AS_STRING\
		DBUS_TYPE_STRING_AS_STRING\
		DBUS_STRUCT_END_CHAR_AS_STRING\
		DBUS_STRUCT_END_CHAR_AS_STRING;

	/* opening array_1 */
	if( ! dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, array_1_sig, &itArray_1) )
	{
		_hosedMessage = true;
		LOG(error) << "first array open_container failure, not enough memory";
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

	try {
		for(const auto & x : depsMap)
		{
			const auto & bin = x.first;   // GLogiK::GKBinary
			const auto & deps = x.second; // GLogiK::GKDepsArray_type

			DBusMessageIter itStruct_1;

			/*
			 * From DBus dbus_message_iter_open_container documentation :
			 *		For structs and dict entries, contained_signature should be NULL;
			 */
			/* opening array_1->struct_1 */
			if( ! dbus_message_iter_open_container(&itArray_1, DBUS_TYPE_STRUCT, nullptr, &itStruct_1) )
			{
				LOG(error) << "first struct open_container failure, not enough memory";
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
			}

			this->appendUInt8(&itStruct_1, toEnumType(bin));
			this->appendUInt64(&itStruct_1, deps.size()); // vector size

			try {
				DBusMessageIter itArray_2;

				// signature = (sss)
				const char array_2_sig[] = \
					DBUS_STRUCT_BEGIN_CHAR_AS_STRING\
					DBUS_TYPE_STRING_AS_STRING\
					DBUS_TYPE_STRING_AS_STRING\
					DBUS_TYPE_STRING_AS_STRING\
					DBUS_STRUCT_END_CHAR_AS_STRING;

				/* opening array_1->struct_1->array_2 */
				if( ! dbus_message_iter_open_container(&itStruct_1, DBUS_TYPE_ARRAY, array_2_sig, &itArray_2) )
				{
					LOG(error) << "second array open_container failure, not enough memory";
					throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
				}

				try {
					for(const auto & dep : deps) // iterating GLogiK::GKDepsArray_type vector
					{
						DBusMessageIter itStruct_2;

						/*
						 * From DBus dbus_message_iter_open_container documentation :
						 *		For structs and dict entries, contained_signature should be NULL;
						 */
						/* opening array_1->struct_1->array_2->struct_2 */
						if( ! dbus_message_iter_open_container(&itArray_2, DBUS_TYPE_STRUCT, nullptr, &itStruct_2) )
						{
							LOG(error) << "second struct open_container failure, not enough memory";
							throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
						}

						try {
							this->appendString(&itStruct_2, dep.getDependency());
							this->appendString(&itStruct_2, dep.getCompileTimeVersion());
							this->appendString(&itStruct_2, dep.getRunTimeVersion());
						}
						catch (const GKDBusMessageWrongBuild & e) {
							/* abandon array_1->struct_1->array_2->struct_2 */
							dbus_message_iter_abandon_container(&itArray_2, &itStruct_2);
							throw;
						}

						/* closing array_1->struct_1->array_2->struct_2 */
						if( ! dbus_message_iter_close_container(&itArray_2, &itStruct_2) ) {
							LOG(error) << "second struct close_container failure, not enough memory";
							throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
						}
					}
				}
				catch (const GKDBusMessageWrongBuild & e) {
					/* abandon array_1->struct_1->array_2 */
					dbus_message_iter_abandon_container(&itStruct_1, &itArray_2);
					throw;
				}

				/* closing array_1->struct_1->array_2 */
				if( ! dbus_message_iter_close_container(&itStruct_1, &itArray_2) )
				{
					LOG(error) << "second array close_container failure, not enough memory";
					throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
				}
			}
			catch (const GKDBusMessageWrongBuild & e) {
				/* abandon array_1->struct_1 */
				dbus_message_iter_abandon_container(&itArray_1, &itStruct_1);
				throw;
			}

			/* closing array_1->struct_1 */
			if( ! dbus_message_iter_close_container(&itArray_1, &itStruct_1) )
			{
				LOG(error) << "first struct close_container failure, not enough memory";
				throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
			}
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_hosedMessage = true;
		/* abandon array_1 */
		dbus_message_iter_abandon_container(iter, &itArray_1);
		throw;
	}

	/* closing array_1 */
	if( ! dbus_message_iter_close_container(iter, &itArray_1) )
	{
		LOG(error) << "first array close_container failure, not enough memory";
		_hosedMessage = true;
		throw GKDBusMessageWrongBuild(TypeBase::appendFailure);
	}

}

/*
 * helper function to get GKBinary
 */
const GLogiK::GKBinary ArgGKBinary::getNextGKBinaryArgument(void)
{
	GK_LOG_FUNC

	GLogiK::GKBinary id = GLogiK::GKBinary::GK_INVALID; // invalid

	try {
		const uint8_t value = ArgUInt8::getNextByteArgument();

		if(value > toEnumType(GLogiK::GKBinary::GK_GUI_QT))
			throw GLogiKExcept("wrong GKBinary value");

		id = static_cast<GLogiK::GKBinary>(value);

		if(id == GLogiK::GKBinary::GK_INVALID)
			throw GLogiKExcept("invalid GKBinary");
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing argument : " << e.what();
		throw GLogiKExcept("get GKBinary argument failed");
	}

	return id;
}

/*
 * helper function to rebuild GKDepsMap_type map
 */
const GLogiK::GKDepsMap_type ArgGKDepsMap::getNextGKDepsMapArgument(void)
{
	GK_LOG_FUNC

	GKLog(trace, "rebuilding GKDepsMap_type map from GKDBus values")

	GLogiK::GKDepsMap_type depsMap;

	try {
		const uint64_t depsMapSize = ArgUInt64::getNextUInt64Argument();

		uint64_t i = 0;
		while(i < depsMapSize)
		{
			GLogiK::GKBinary bin = ArgGKBinary::getNextGKBinaryArgument();
			GLogiK::GKDepsArray_type depsArray;

			const uint64_t depsArraySize = ArgUInt64::getNextUInt64Argument();
			uint64_t j = 0;
			while(j < depsArraySize)
			{
				const std::string dependency = ArgString::getNextStringArgument();
				const std::string compileTime = ArgString::getNextStringArgument();
				const std::string runTime = ArgString::getNextStringArgument();

				depsArray.emplace_back(dependency, compileTime, runTime);
				++j;
			}

			depsMap[bin] = depsArray;

			++i;
		}
	}
	catch ( const EmptyContainer & e ) {
		LOG(warning) << "missing argument : " << e.what();
		throw GLogiKExcept("rebuilding GKDepsMap_type map failed");
	}

	return depsMap;
}

} // namespace NSGKDBus
