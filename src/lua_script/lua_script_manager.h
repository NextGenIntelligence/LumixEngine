#pragma once


#include "engine/array.h"
#include "engine/resource.h"
#include "engine/resource_manager_base.h"
#include "engine/string.h"


namespace Lumix
{


class LuaScript : public Resource
{
public:
	LuaScript(const Path& path,
			  ResourceManager& resource_manager,
			  IAllocator& allocator);
	virtual ~LuaScript();

	void unload() override;
	bool load(FS::IFile& file) override;
	const char* getSourceCode() const { return m_source_code.c_str(); }

private:
	string m_source_code;
};


class LuaScriptManager : public ResourceManagerBase
{
public:
	explicit LuaScriptManager(IAllocator& allocator);
	~LuaScriptManager();

protected:
	Resource* createResource(const Path& path) override;
	void destroyResource(Resource& resource) override;

private:
	IAllocator& m_allocator;
};


} // namespace Lumix