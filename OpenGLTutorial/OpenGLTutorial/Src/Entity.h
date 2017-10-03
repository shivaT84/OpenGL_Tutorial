/**
* @file Entity.h
*/

#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED
#include <GL/glew.h>
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <functional>

namespace Entity {

	class Buffer;
	typedef std::shared_ptr<Buffer> BufferPtr; ///< エンティティバッファポインタ型.

	/**
	* エンティティ.
	*/
	class Entity{
		friend class Buffer;

	public:
		/// 状態更新関数.
		typedef std::function<void(Entity&, void*, double, const glm::mat4&, const glm::mat4&)> UpdateFuncType;

		void Position(const glm::vec3& pos) { position = pos; }
		const glm::vec3& Position() const { return position; }
		void Rotation(const glm::quat& rot) { rotation = rot; }
		const glm::quat& Rotation() const { return rotation; }
		void Scale(const glm::vec3& s) { scale = s; }
		const glm::vec3& Scale() const { return scale; }
		void Velocity(const glm::vec3& v) { velocity = v; }
		const glm::vec3& Velocity() const { return velocity; }
		void UpdateFunc(const UpdateFuncType& func) { updateFunc = func; }
		const UpdateFuncType& UpdateFunc() const { return updateFunc; }

		glm::mat4 TRSMatrix() const;

	private:
		Entity() = default;
		~Entity() = default;
		Entity(const Entity&) = default;
		Entity& operator=(const Entity&) = default;

	private:
		glm::vec3 position; ///< 座標.
		glm::vec3 scale = glm::vec3(1, 1, 1); ///< 大きさ.
		glm::quat rotation; ///< 回転.
		glm::vec3 velocity; ///< 速度.
		UpdateFuncType updateFunc; ///< 状態更新関数.
		Mesh::MeshPtr mesh; ///< エンティティを描画するときに使われるメッシュデータ.
		TexturePtr texture; ///< エンティティを描画するときに使われるテクスチャ.
		Shader::ProgramPtr program; ///< エンティティを描画するときに使われるシェーダ.
		GLintptr uboOffset; ///< UBOのエンティティ用領域へのバイトオフセット.
		bool isActive = false; ///< アクティブなエンティティならtrue、非アクティブならfalse.
	};

	/**
	* エンティティバッファ.
	*/
	class Buffer{
	public:
		static BufferPtr Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* name);

		Entity* AddEntity(const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr& t, const Shader::ProgramPtr& p, Entity::UpdateFuncType func);
		void RemoveEntity(Entity* entity);
		void Update(double delta, const glm::mat4& matView, const glm::mat4& matProj);
		void Draw(const Mesh::BufferPtr& meshBuffer) const;

	private:
		Buffer() = default;
		~Buffer() = default;
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

	private:
		/// エンティティ用リンクリスト.
		struct Link {
			void Insert(Link* e);
			void Remove();
			Link* prev = this;
			Link* next = this;
		};
		/// リンクつきエンティティ.
		struct LinkEntity : public Link, public Entity {};
		/// リンク付きエンティティ配列の削除関数.
		struct EntityArrayDeleter { void operator()(LinkEntity* p) { delete[] p; } };

		std::unique_ptr<LinkEntity[], EntityArrayDeleter> buffer; ///< エンティティの配列. 
		size_t bufferSize; ///< エンティティの総数.
		Link freeList; ///< 未使用のエンティティのリンクリスト.
		Link activeList; ///< 使用中のエンティティのリンクリスト.
		GLsizeiptr ubSizePerEntity; ///< 各エンティティが使える Uniform Buffer のバイト数.
		UniformBufferPtr ubo; ///< エンティティ用 UBO.
		Link* itrUpdate = nullptr; ///< Update と RemoveEntity の相互作用に対応するためのイテレータ.
	};
}

#endif // ENTITY_H_INCLUDED