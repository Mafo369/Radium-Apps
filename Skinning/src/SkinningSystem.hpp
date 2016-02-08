#ifndef SKINPLUGIN_SKINNING_SYSTEM_HPP_
#define SKINPLUGIN_SKINNING_SYSTEM_HPP_

#include <SkinningPlugin.hpp>

#include <Engine/Entity/System.hpp>

#include <Core/Tasks/TaskQueue.hpp>
#include <Engine/Assets/FileData.hpp>
#include <Engine/Assets/HandleData.hpp>
#include <Engine/Entity/Entity.hpp>
#include <SkinningTask.hpp>
#include <SkinningComponent.hpp>
namespace SkinningPlugin
{

    class SKIN_PLUGIN_API SkinningSystem :  public Ra::Engine::System
    {
    public:
        SkinningSystem(){}
        virtual void generateTasks( Ra::Core::TaskQueue* taskQueue,
                                    const Ra::Engine::FrameInfo& frameInfo ) override
        {
            for (const auto& compEntry : m_components)
            {
                SkinningComponent* comp = static_cast<SkinningComponent*>(compEntry.second);
                SkinnerTask* task = new SkinnerTask(comp);

                Ra::Core::TaskQueue::TaskId taskId = taskQueue->registerTask(task);
                taskQueue->addDependency("AnimatorTask", taskId);
            }

        }



        void handleAssetLoading( Ra::Engine::Entity* entity, const Ra::Asset::FileData* fileData) override
        {

            auto geomData = fileData->getGeometryData();
            auto skelData = fileData->getHandleData();

            if ( geomData.size() > 0  && skelData.size() > 0 )
            {
                for (const auto& skel : skelData)
                {
                    SkinningComponent* component = new SkinningComponent("SkC_" + skel->getName());
                    entity->addComponent( component );
                    component->handleWeightsLoading(skel);
                    registerComponent(entity, component);
                }
            }
        }

    };
}


#endif // ANIMPLUGIN_SKINNING_SYSTEM_HPP_
