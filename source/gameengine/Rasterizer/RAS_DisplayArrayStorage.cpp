#include "RAS_DisplayArrayStorage.h"
#include "RAS_StorageVbo.h"

RAS_DisplayArrayStorage::RAS_DisplayArrayStorage(RAS_IDisplayArray *array)
	:m_vbo(new RAS_StorageVbo(array))
{
}

RAS_DisplayArrayStorage::~RAS_DisplayArrayStorage()
{
}

void RAS_DisplayArrayStorage::IndexPrimitives()
{
	m_vbo->IndexPrimitives();
}

void RAS_DisplayArrayStorage::IndexPrimitivesInstancing(unsigned int numslots)
{
	m_vbo->IndexPrimitivesInstancing(numslots);
}

void RAS_DisplayArrayStorage::IndexPrimitivesBatching(const std::vector<void *>& indices, const std::vector<int>& counts)
{
	m_vbo->IndexPrimitivesBatching(indices, counts);
}
