#include <FancyMeshComponent.hpp>

#include <iostream>

#include <Core/String/StringUtils.hpp>
#include <Core/Mesh/MeshUtils.hpp>

#include <Core/Geometry/Normal/Normal.hpp>

#include <Engine/Renderer/RenderObject/RenderObjectManager.hpp>
#include <Engine/Entity/ComponentMessenger.hpp>

#include <Engine/Renderer/Mesh/Mesh.hpp>
#include <Engine/Renderer/RenderObject/RenderObject.hpp>
#include <Engine/Renderer/RenderObject/RenderObjectTypes.hpp>
#include <Engine/Renderer/RenderTechnique/RenderTechnique.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderConfiguration.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgramManager.hpp>
#include <Engine/Renderer/RenderObject/Primitives/DrawPrimitives.hpp>

#include <Engine/Assets/FileData.hpp>
#include <Engine/Assets/GeometryData.hpp>

namespace FancyMeshPlugin
{
    FancyMeshComponent::FancyMeshComponent(const std::string& name , bool deformable)
        : Ra::Engine::Component( name  ) , m_deformable(deformable)
    {
    }

    FancyMeshComponent::~FancyMeshComponent()
    {
        // TODO(Charly): Should we ask the RO manager to delete our render object ?
    }

    void FancyMeshComponent::initialize()
    {
    }

    void FancyMeshComponent::addMeshRenderObject( const Ra::Core::TriangleMesh& mesh, const std::string& name )
    {
        Ra::Engine::RenderTechnique* technique = new Ra::Engine::RenderTechnique;
        technique->material = new Ra::Engine::Material( "Default" );
        technique->shaderConfig = Ra::Engine::ShaderConfiguration( "Default", "../Shaders" );

        addMeshRenderObject(mesh, name, technique);
    }

    void FancyMeshComponent::addMeshRenderObject( const Ra::Core::TriangleMesh& mesh,
                                                  const std::string& name,
                                                  Ra::Engine::RenderTechnique* technique )
    {
        setupIO(name);

        Ra::Engine::RenderObject* renderObject = new Ra::Engine::RenderObject( name, this, Ra::Engine::RenderObjectType::FANCY );
        renderObject->setVisible( true );
        renderObject->setRenderTechnique( technique );

        std::shared_ptr<Ra::Engine::Mesh> displayMesh( new Ra::Engine::Mesh( name ) );
        displayMesh->loadGeometry( mesh );
        renderObject->setMesh( displayMesh );
        addRenderObject( renderObject );
    }

    void FancyMeshComponent::handleMeshLoading( const Ra::Asset::GeometryData* data )
    {
        std::string name( m_name );
        name.append( "_" + data->getName() );

        std::string roName = name;
        roName.append( "_RO" );

        std::string meshName = name;
        meshName.append( "_Mesh" );

        std::string matName = name;
        matName.append( "_Mat" );

        m_contentName = data->getName();

        Ra::Engine::RenderObject* renderObject = new Ra::Engine::RenderObject( roName, this, Ra::Engine::RenderObjectType::FANCY );
        renderObject->setVisible( true );

        std::shared_ptr<Ra::Engine::Mesh> displayMesh( new Ra::Engine::Mesh( meshName ) );

        Ra::Core::TriangleMesh mesh;
        for( const auto& v : data->getVertices() )
        {
            mesh.m_vertices.push_back( data->getFrame() * v );
        }
        for( const auto& face : data->getFaces() ) {
            mesh.m_triangles.push_back( {uint(face[0]), uint(face[1]), uint(face[2]) } );
        }


        Ra::Core::Geometry::uniformNormal( mesh.m_vertices, mesh.m_triangles, mesh.m_normals );

        setupIO( data->getName());

        displayMesh->loadGeometry(mesh);

        Ra::Core::Vector3Array tangents;
        Ra::Core::Vector3Array bitangents;
        Ra::Core::Vector3Array texcoords;

        Ra::Core::Vector4Array colors;

        for ( const auto& v : data->getTangents() )     tangents.push_back( v );
        for ( const auto& v : data->getBiTangents() )   bitangents.push_back( v );
        for ( const auto& v : data->getTexCoords() )    texcoords.push_back( v );
        for ( const auto& v : data->getColors() )       colors.push_back( v );



        displayMesh->addData( Ra::Engine::Mesh::VERTEX_TANGENT, tangents );
        displayMesh->addData( Ra::Engine::Mesh::VERTEX_BITANGENT, bitangents );
        displayMesh->addData( Ra::Engine::Mesh::VERTEX_TEXCOORD, texcoords );
        displayMesh->addData( Ra::Engine::Mesh::VERTEX_COLOR, colors );
        // FIXME(Charly): Should not weights be part of the geometry ?
        //        mesh->addData( Ra::Engine::Mesh::VERTEX_WEIGHTS, meshData.weights );

        renderObject->setMesh( displayMesh );

        m_meshIndex = addRenderObject(renderObject);

        Ra::Engine::RenderTechnique* rt = new Ra::Engine::RenderTechnique;
        Ra::Engine::Material* mat = new Ra::Engine::Material( matName );
        auto m = data->getMaterial();
        if ( m.hasDiffuse() )   mat->setKd( m.m_diffuse );
        if ( m.hasSpecular() )  mat->setKs( m.m_specular );
        if ( m.hasShininess() ) mat->setNs( m.m_shininess );
        //if ( m.hasDiffuseTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_DIFFUSE, m.m_texDiffuse );
        //if ( m.hasSpecularTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_SPECULAR, m.m_texSpecular );
        //if ( m.hasShininessTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_SHININESS, m.m_texShininess );
        //if ( m.hasOpacityTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_ALPHA, m.m_texOpacity );
        //if ( m.hasNormalTexture() ) mat->addTexture( Ra::Engine::Material::TextureType::TEX_NORMAL, m.m_texNormal );

        rt->material = mat;
        rt->shaderConfig = Ra::Engine::ShaderConfiguration( "BlinnPhong", "../Shaders" );

        renderObject->setRenderTechnique( rt );

    }

    Ra::Core::Index FancyMeshComponent::getRenderObjectIndex() const
    {
        return m_meshIndex;
    }

    const Ra::Core::TriangleMesh& FancyMeshComponent::getMesh() const
    {
        return getDisplayMesh().getGeometry();
    }

    void FancyMeshComponent::setupIO(const std::string& id)
    {
        Ra::Engine::ComponentMessenger::GetterCallback cbOut = std::bind( &FancyMeshComponent::getMeshOutput, this );
        Ra::Engine::ComponentMessenger::getInstance()->registerOutput<Ra::Core::TriangleMesh>( getEntity(), this, id, cbOut);

        if( m_deformable)
        {
            Ra::Engine::ComponentMessenger::SetterCallback cbIn = std::bind( &FancyMeshComponent::setMeshInput, this, std::placeholders::_1 );
            Ra::Engine::ComponentMessenger::getInstance()->registerInput<Ra::Core::TriangleMesh>( getEntity(), this, id, cbIn);

            Ra::Engine::ComponentMessenger::ReadWriteCallback vRW = std::bind( &FancyMeshComponent::getVerticesRw, this);
            Ra::Engine::ComponentMessenger::getInstance()->registerReadWrite<Ra::Core::Vector3Array>( getEntity(), this, id+"v", vRW);

            Ra::Engine::ComponentMessenger::ReadWriteCallback nRW = std::bind( &FancyMeshComponent::getNormalsRw, this);
            Ra::Engine::ComponentMessenger::getInstance()->registerReadWrite<Ra::Core::Vector3Array>( getEntity(), this, id+"n", nRW);

            Ra::Engine::ComponentMessenger::ReadWriteCallback tRW = std::bind( &FancyMeshComponent::getTrianglesRw, this);
            Ra::Engine::ComponentMessenger::getInstance()->registerReadWrite<Ra::Core::Vector3Array>( getEntity(), this, id+"t", tRW);
        }

    }

    const Ra::Engine::Mesh& FancyMeshComponent::getDisplayMesh() const
    {
        return *(getRoMgr()->getRenderObject(getRenderObjectIndex())->getMesh());
    }

    Ra::Engine::Mesh& FancyMeshComponent::getDisplayMesh()
    {
        return *(getRoMgr()->getRenderObject(getRenderObjectIndex())->getMesh());
    }

    const void* FancyMeshComponent::getMeshOutput() const
    {
        return &(getMesh());
    }

    void FancyMeshComponent::setMeshInput(const void *meshptr)
    {

        CORE_ASSERT( meshptr, " Input is null");
        CORE_ASSERT( m_deformable, "Mesh is not deformable");

        Ra::Core::TriangleMesh mesh = *(static_cast<const Ra::Core::TriangleMesh*>(meshptr));

        Ra::Engine::Mesh& displayMesh = getDisplayMesh();
        displayMesh.loadGeometry( mesh );
    }

    void* FancyMeshComponent::getVerticesRw()
    {
        getDisplayMesh().setDirty( Ra::Engine::Mesh::VERTEX_POSITION);
        return &(getDisplayMesh().getGeometry().m_vertices);
    }

    void* FancyMeshComponent::getNormalsRw()
    {
        getDisplayMesh().setDirty( Ra::Engine::Mesh::VERTEX_NORMAL);
        return &(getDisplayMesh().getGeometry().m_normals);
    }

    void* FancyMeshComponent::getTrianglesRw()
    {
        getDisplayMesh().setDirty( Ra::Engine::Mesh::INDEX);
        return &(getDisplayMesh().getGeometry().m_triangles);
    }

    void FancyMeshComponent::rayCastQuery( const Ra::Core::Ray& r) const
    {
        auto result  = Ra::Core::MeshUtils::castRay( getMesh(), r );
        int tidx = result.m_hitTriangle;
        if (tidx >= 0)
        {
            LOG(logINFO) << " Hit triangle " << tidx;
            LOG(logINFO) << " Nearest vertex " << result.m_nearestVertex;
        }
    }

} // namespace FancyMeshPlugin
