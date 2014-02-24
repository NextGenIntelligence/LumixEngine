#include "graphics/pose.h"
#include "core/matrix.h"
#include "core/quat.h"
#include "core/vec3.h"


namespace Lux
{


Pose::Pose()
{
	m_positions = 0;
	m_rotations = 0;
	m_count = 0;
}


Pose::~Pose()
{
	LUX_DELETE_ARRAY(m_positions);
	LUX_DELETE_ARRAY(m_rotations);
}


void Pose::resize(int count)
{
	LUX_DELETE_ARRAY(m_positions);
	LUX_DELETE_ARRAY(m_rotations);
	m_count = count;
	m_positions = LUX_NEW_ARRAY(Vec3, count);
	m_rotations = LUX_NEW_ARRAY(Quat, count);
}


void Pose::setMatrices(Matrix* mtx) const
{
	for(int i = 0, c = m_count; i < c; ++i)
	{
		m_rotations[i].toMatrix(mtx[i]);
		mtx[i].translate(m_positions[i]);
	}
}


} // ~namespace Lux