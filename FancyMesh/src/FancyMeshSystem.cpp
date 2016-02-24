#include <FancyMeshSystem.hpp>

#include <Core/String/StringUtils.hpp>
#include <Core/Tasks/Task.hpp>
#include <Core/Tasks/TaskQueue.hpp>

#include <Engine/RadiumEngine.hpp>
#include <Engine/Entity/Entity.hpp>
#include <Engine/FrameInfo.hpp>
#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Assets/FileData.hpp>
#include <Engine/Assets/GeometryData.hpp>
#include <Engine/Managers/ComponentMessenger/ComponentMessenger.hpp>

#include <FancyMeshComponent.hpp>

namespace FancyMeshPlugin
{

    FancyMeshSystem::FancyMeshSystem()
        : Ra::Engine::System()
    {
    }

    FancyMeshSystem::~FancyMeshSystem()
    {
    }

    void FancyMeshSystem::handleAssetLoading( Ra::Engine::Entity* entity, const Ra::Asset::FileData* fileData )
    {
        auto geomData = fileData->getGeometryData();

        uint id = 0;

        for ( const auto& data : geomData )
        {
            std::string componentName = "FMC_" + entity->getName() + std::to_string( id++ );
            FancyMeshComponent * comp = new FancyMeshComponent( componentName );
            entity->addComponent( comp );
            comp->handleMeshLoading(data);
            registerComponent( entity, comp );
        }
    }

    void FancyMeshSystem::generateTasks( Ra::Core::TaskQueue* taskQueue, const Ra::Engine::FrameInfo& frameInfo )
    {
        // Do nothing, as this system only displays meshes.
    }

    FancyMeshComponent* FancyMeshSystem::makeFancyMeshFromGeometry(
                    const Ra::Core::TriangleMesh& mesh, const std::string& name,
                    Ra::Engine::RenderTechnique* technique )
    {
        FancyMeshComponent* comp = new FancyMeshComponent(name);
        if ( technique )
        {
            comp->addMeshRenderObject( mesh, "Mesh RenderObject", technique );
        }
        else
        {
            comp->addMeshRenderObject( mesh, "Mesh RenderObject" );
        }
        return comp;
    }

} // namespace FancyMeshPlugin
