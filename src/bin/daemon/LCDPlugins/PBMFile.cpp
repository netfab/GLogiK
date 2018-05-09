/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include "lib/utils/utils.h"

#include "PBMFile.h"

namespace GLogiK
{

using namespace NSGKUtils;

PBMFile::PBMFile() {
}

PBMFile::~PBMFile() {
}

void PBMFile::readPBM(
	const std::string & path,
	const unsigned int expected_width,
	const unsigned int expected_height)
{
	std::ifstream pbm;
	pbm.exceptions(std::ifstream::failbit | std::ifstream::badbit);

#if DEBUGGING_ON
	LOG(DEBUG2) << "opening : " << path;
#endif

	try {
		pbm.open(path, std::ifstream::in|std::ifstream::binary);

		std::string magic;
		unsigned int width, height = 0;

		parsePBMHeader(pbm, magic, width, height);

		if( magic != "P4" or width != expected_width or height != expected_height )
			throw GLogiKExcept("wrong PBM header");

		extractPBMData(pbm, this->pbm_data_);

		pbm.close();
	}
	catch (const std::ios_base::failure & e) {
		LOG(ERROR) << "error opening/reading/closing PBM file : " << e.what();
		this->closePBM(pbm);
		throw GLogiKExcept("PBM ifstream error");
	}
	/*
	 * catch std::ios_base::failure on buggy compilers
	 * should be fixed with gcc >= 7.0
	 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	 */
	catch( const std::exception & e ) {
		LOG(ERROR) << "(buggy exception) error opening/reading/closing PBM file : " << e.what();
		this->closePBM(pbm);
		throw GLogiKExcept("PBM ifstream error");
	}

	if( pbm.is_open() )
		this->closePBM(pbm);
}

const PBMDataArray & PBMFile::getPBMData(void)
{
	return this->pbm_data_;
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
	unsigned int & width,
	unsigned int & height)
{
	const char c = 0x0a;
	const char space = 0x20;

	std::getline(pbm, magic, c);
	std::string ex;
	std::getline(pbm, ex, space);
	width = to_uint(ex);
	std::getline(pbm, ex, c);
	height = to_uint(ex);

#if DEBUGGING_ON
	LOG(DEBUG2)	<< "magic: " << magic
				<< " - width: " << width
				<< " - height: " << height << "\n";
#endif
}

void PBMFile::extractPBMData(
	std::ifstream & pbm,
	PBMDataArray & pbm_data)
{
	pbm.read(reinterpret_cast<char*>(&pbm_data.front()), pbm_data.size());

#if DEBUGGING_ON
	LOG(DEBUG2)	<< "extracted bytes: " << pbm.gcount()
				<< " - expected: " << pbm_data.size() << std::endl;
#endif

	if( pbm_data.size() != pbm.gcount() )
		throw GLogiKExcept("unexpected numbers of bytes read");

	// checking that we really reached the EoF
	pbm.ignore();
	if( ! pbm.eof() )
		throw GLogiKExcept("EoF NOT reached, that is unexpected");
}

} // namespace GLogiK

