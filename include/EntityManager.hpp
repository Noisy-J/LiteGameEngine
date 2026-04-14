#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include "Components.hpp"
#include "ResourceManager.hpp"

using Entity = uint32_t;

class Scene {
public:
	Entity createEntity() {
		return lastEntity++;
	}

	std::unordered_map<Entity, TransformComponent> transforms;
	std::unordered_map<Entity, SpriteComponent> sprites;
	std::unordered_map<Entity, VelocityComponent> velocities;
	std::unordered_map<Entity, PhysicsComponent> physics;

	void setTexture(Entity entity, sf::Texture& tex) {
		auto& sc = sprites[entity];
		sc.sprite.emplace(tex);
		// По умолчанию ставим центр в середину
		sc.sprite->setOrigin({ tex.getSize().x / 2.f, tex.getSize().y / 2.f });
	}

	void setOrigin(Entity entity, int Type) {
		auto& sc = sprites[entity];
		auto& rect = sprites[entity].sprite->getTextureRect();
		// По умолчанию ставим центр в середину
		sc.sprite->setOrigin({ rect.size.x / 2.f, rect.size.y / 2.f });
	}

	void setPosition(Entity entity, sf::Vector2f pos) {
		auto& tf = transforms[entity];
		tf.Pos = pos;
	}

	void setRotation(Entity entity, float rot) {
		auto& tf = transforms[entity];
		tf.Rot = sf::degrees(rot);
	}

	void setScale(Entity entity, sf::Vector2f scl) {
		auto& tf = transforms[entity];
		tf.Scale = scl;
	}

	void setVelocity(Entity entity, sf::Vector2f vel) {
		auto& vlc = velocities[entity];
		vlc.Velocity = vel;
	}

private:
	Entity lastEntity = 0;
};
