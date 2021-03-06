/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include "PBMFile.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

PBMFile::PBMFile()
{
}

PBMFile::~PBMFile()
{
}

void PBMFile::readPBM(
	const std::string & PBMPath,
	PixelsData & PBMData,
	const uint16_t PBMWidth,
	const uint16_t PBMHeight)
{
	std::ifstream pbm;
	pbm.exceptions(std::ifstream::failbit | std::ifstream::badbit);

#if DEBUGGING_ON
	LOG(DEBUG2) << "opening : " << PBMPath;
#endif

	try {
		try {
			pbm.open(PBMPath, std::ifstream::in|std::ifstream::binary);

			std::string magic;
			uint16_t width, height = 0;

			PBMFile::parsePBMHeader(pbm, magic, width, height);

			if( magic != "P4" or width != PBMWidth or height != PBMHeight ) {
				throw GLogiKExcept("wrong PBM header");
			}

			PBMFile::extractPBMData(pbm, PBMData);

			pbm.close();
		}
		catch (const std::ios_base::failure & e) {
			LOG(ERROR) << "error opening/reading/closing PBM file : " << e.what();
			throw GLogiKExcept("PBM ifstream error");
		}
	}
	catch (const GLogiKExcept & e) {
		PBMFile::closePBM(pbm);
		throw;
	}

	if( pbm.is_open() )
		PBMFile::closePBM(pbm);
}

void PBMFile::closePBM(std::ifstream & pbm)
{
	/* disable exceptions */
	pbm.exceptions(std::ifstream::goodbit);
	pbm.close();
}

void PBMFile::parsePBMHeader(
	std::ifstream & pbm,
	std::string & magic,
	uint16_t & width,
	uint16_t & height)
{
	const char white = 0x0a;
	const char space = 0x20;
	const char comment = 0x23;

	std::getline(pbm, magic, white);

	char c = comment;

	std::string ex;

	/* searching and skipping comments */
	while( c == comment ) {
		pbm.get(c);
		if( c == comment) {
			std::getline(pbm, ex, white);
#if DEBUGGING_ON
			LOG(DEBUG3)	<< "skipped PBM comment";
#endif
		}
		else pbm.unget();
	}

	std::getline(pbm, ex, space);
	width = toUInt(ex);
	std::getline(pbm, ex, white);
	height = toUInt(ex);

#if DEBUGGING_ON
	LOG(DEBUG2)	<< "magic: " << magic
				<< " - width: " << width
				<< " - height: " << height << "\n";
#endif
}

void PBMFile::extractPBMData(
	std::ifstream & pbm,
	PixelsData & PBMData)
{
	pbm.read(reinterpret_cast<char*>(&PBMData.front()), PBMData.size());

#if DEBUGGING_ON
	LOG(DEBUG2)	<< "extracted bytes: " << pbm.gcount()
				<< " - expected: " << PBMData.size() << std::endl;
#endif

	if( PBMData.size() != static_cast<PixelsData::size_type>( pbm.gcount() ) )
		throw GLogiKExcept("unexpected numbers of bytes read");

	// checking that we really reached the EoF
	pbm.ignore();
	if( ! pbm.eof() )
		throw GLogiKExcept("EoF NOT reached, that is unexpected");
}

} // namespace GLogiK

