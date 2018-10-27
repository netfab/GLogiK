
#include "deviceFile.hpp"

namespace GLogiK
{

DeviceFile::DeviceFile()
	:	_status("unknown"),
		_vendor("unknown"),
		_model("unknown"),
		_filePath("none")
{
}

DeviceFile::~DeviceFile()
{
}

const std::string & DeviceFile::getStatus(void) const
{
	return _status;
}

const std::string & DeviceFile::getVendor(void) const
{
	return _vendor;
}

const std::string & DeviceFile::getModel(void) const
{
	return _model;
}

const std::string & DeviceFile::getConfigFilePath(void) const
{
	return _filePath;
}

void DeviceFile::setStatus(const std::string & status)
{
	_status = status;
}

void DeviceFile::setVendor(const std::string & vendor)
{
	_vendor = vendor;
}

void DeviceFile::setModel(const std::string & model)
{
	_model = model;
}

void DeviceFile::setConfigFilePath(const std::string & path)
{
	_filePath = path;
}

} // namespace GLogiK
