#include "FancyMeshSystem.hpp"

#include <Core/String/StringUtils.hpp>
#include <Core/Tasks/Task.hpp>
#include <Core/Tasks/TaskQueue.hpp>

#include <Engine/RadiumEngine.hpp>
#include <Engine/Entity/Entity.hpp>
#include <Engine/Entity/FrameInfo.hpp>
#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Assets/FileData.hpp>
#include <Engine/Assets/GeometryData.hpp>

#include "FancyMeshComponent.hpp"

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

        for( const auto& data : geomData ) {
            // FIXME(Charly): One component of a given type by entity ?
            FancyMeshComponent* component = static_cast<FancyMeshComponent*>(addComponentToEntity(entity));
            component->handleMeshLoading( data );
        }
    }

    Ra::Engine::Component* FancyMeshSystem::addComponentToEntityInternal( Ra::Engine::Entity* entity, uint id )
    {
        std::string componentName = "FMC_" + entity->getName() + std::to_string( id );
        FancyMeshComponent* component = new FancyMeshComponent( componentName );

        return component;
    }

    void FancyMeshSystem::generateTasks( Ra::Core::TaskQueue* taskQueue, const Ra::Engine::FrameInfo& frameInfo )
    {
        // Do nothing, as this system only displays meshes.
    }

    FancyMeshComponent* FancyMeshSystem::addFancyMeshToEntity( Ra::Engine::Entity* entity,
                                                               const Ra::Core::TriangleMesh& mesh )
    {
        FancyMeshComponent* comp = static_cast<FancyMeshComponent*>( addComponentToEntity( entity ) );
        comp->addMeshRenderObject( mesh, "Mesh RenderObject" );
        return comp;
    }

    FancyMeshComponent* FancyMeshSystem::addFancyMeshToEntity( Ra::Engine::Entity* entity,
                                                               const Ra::Core::TriangleMesh& mesh,
                                                               Ra::Engine::RenderTechnique* technique )
    {
        FancyMeshComponent* comp = static_cast<FancyMeshComponent*>( addComponentToEntity( entity ) );
        comp->addMeshRenderObject( mesh, "Mesh RenderObject", technique );
        return comp;
    }

} // namespace FancyMeshPlugin
