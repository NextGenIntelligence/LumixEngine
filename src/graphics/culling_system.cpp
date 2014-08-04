#include "core/lumix.h"
#include "culling_system.h"

#include "core/array.h"
#include "core/frustum.h"
#include "core/sphere.h"

#include "core/mtjd/group.h"
#include "core/mtjd/manager.h"
#include "core/mtjd/job.h"

namespace Lumix
{
	static void doCulling(
		const Sphere* __restrict  start,
		const Sphere* __restrict  end,
		const int* __restrict indexes, 
		const Frustum* __restrict frustum,
		int* __restrict results
		)
	{
		for (const Sphere* sphere = start; sphere <= end; sphere++, indexes++, results++)
		{
			if (frustum->sphereInFrustum(sphere->m_position, sphere->m_radius))
			{
				*results = *indexes;
			}
			else
			{
				*results = -1;
			}
		}
	}

	class CullingJob : public MTJD::Job
	{
		friend class ResultsCollectorJob;

	public:
		CullingJob(const CullingSystem::InputSpheres& spheres, const CullingSystem::Indexes& indexes, CullingSystem::Results& results, int start, int end, const Frustum& frustum, MTJD::Manager& manager)
			: Job(true, MTJD::Priority::Default, false, manager)
			, m_spheres(spheres)
			, m_indexes(indexes)
			, m_results(results)
			, m_start(start)
			, m_end(end)
			, m_frustum(frustum)
		{
			setJobName("CullingJob");
			m_results.reserve(end - start);
		}

		virtual ~CullingJob()
		{
		}

		virtual void execute() override
		{
			doCulling(&m_spheres[m_start], &m_spheres[m_end], &m_indexes[m_start], &m_frustum, &m_results[m_start]);
		}

	private:
		const CullingSystem::InputSpheres& m_spheres;
		const CullingSystem::Indexes& m_indexes;
		CullingSystem::Results& m_results;
		int m_start;
		int m_end;
		const Frustum& m_frustum;

	};

	class CullingSystemImpl : public CullingSystem
	{
	public:
		CullingSystemImpl(MTJD::Manager& mtjd_manager)
			: m_mtjd_manager(mtjd_manager)
			, m_sync_point(true)
		{
		}

		virtual ~CullingSystemImpl()
		{

		}



		virtual const Results& getResult() override
		{

			return m_result;
		}


		virtual const Results& getResultAsync() override
		{
			m_sync_point.sync();
			return m_result;
		}

		virtual void cullToFrustum(const Frustum& frustum) override
		{

			int count = m_spheres.size();
			m_result.clear();
			m_result.resize(count);

			doCulling(&m_spheres[0], &m_spheres[count - 1], &m_indexes[0], &frustum, &m_result[0]);
		}

		// ficni async, mal by si do jobu poslat culling system a zneho potrebne veci precitat.
		// momentalne je tam chyba a posielaju sa sracky
		virtual void cullToFrustumAsync(const Frustum& frustum) override
		{
			int count = m_spheres.size();
			m_result.clear();
			m_result.resize(count);

			int cpu_count = m_mtjd_manager.getCpuThreadsCount();
			cpu_count = Math::minValue(count, cpu_count);
			ASSERT(cpu_count > 0);

			int step = count / cpu_count;
			int i = 0;
			for (; i < cpu_count - 1; i++)
			{
				CullingJob* cj = LUMIX_NEW(CullingJob)(m_spheres, m_indexes, m_result, i * step, (i + 1) * step - 1, frustum, m_mtjd_manager);
				cj->addDependency(&m_sync_point);
				m_mtjd_manager.schedule(cj);
			}

			CullingJob* cj = LUMIX_NEW(CullingJob)(m_spheres, m_indexes, m_result, i * step, count - 1, frustum, m_mtjd_manager);
			cj->addDependency(&m_sync_point);

			m_mtjd_manager.schedule(cj);
		}


		virtual void addStatic(const Sphere& sphere, int index) override
		{
			m_spheres.push(sphere);
			m_indexes.push(index);
		}


		virtual void insert(const InputSpheres& spheres) override
		{
			for (int i = 0; i < spheres.size(); i++)
			{
				m_spheres.push(spheres[i]);
				m_indexes.push(i);
			}
		}

	private:
		InputSpheres	m_spheres;
		Indexes			m_indexes;
		Results			m_result;

		MTJD::Manager& m_mtjd_manager;
		MTJD::Group m_sync_point;
	};


	CullingSystem* CullingSystem::create(MTJD::Manager& mtjd_manager)
	{
		return LUMIX_NEW(CullingSystemImpl)(mtjd_manager);
	}


	void CullingSystem::destroy(CullingSystem& culling_system)
	{
		LUMIX_DELETE(&culling_system);
	}
}