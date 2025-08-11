#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>

#include "maincamera.hpp"
#include "move_tool.hpp"
#include "sun_tool.hpp"
#include "rotate_tool.hpp"
#include "player.hpp"
#include "util.hpp"
#include "world.hpp"

Player::Player(std::string username, Vector3 position) : username_(username), hitbox_({0.0f,0.0f,0.0f}, {1.0f, 2.0f, 1.0f}, 1.0f, WHITE) {
    speed_ = 4.0f;
    pickup_range_ = 3.0f;
    online_ = false;
    set_position(position);
    auto head = std::make_unique<Cube>(Vector3{0, 1.0f, 0}, Vector3{0.5f, 0.5f, 0.5f}, 1.0f,PINK);
    add_to_model(std::move(head));
    selected_item_ = nullptr;
};

Player::Player(std::string data) : hitbox_({0.0f,0.0f,0.0f}, {1.0f, 2.0f, 1.0f}, 1.0f, WHITE) {
    speed_ = 4.0f;
    pickup_range_ = 3.0f;
    std::vector<std::string> split = split_string(data);
    assert(split[0] == "Player" && split.size() == 8);
    username_ = split[1];
    set_position(Vector3{std::stof(split[2]), std::stof(split[3]), std::stof(split[4])});
    online_ = std::stoi(split[5]) == 1;
    if (split[6] != "null_item") {
        if (get_first_word(split[6]) == "MoveTool") {
            selected_item_ = std::make_shared<MoveTool>(split[6]);
        } else if (get_first_word(split[6]) == "SunTool") {
            selected_item_ = std::make_shared<SunTool>(split[6]);
        } else if (get_first_word(split[6]) == "RotateTool") {
            selected_item_ = std::make_shared<RotateTool>(split[6]);
        }
    }
    std::vector<std::string> model_objects = split_string(split[7]);
    for (std::string object_data : model_objects) {
        if (get_first_word(object_data) == "Cube") {
            add_to_model(std::make_unique<Cube>(object_data));
        }
    }
};

Player::~Player() {
    shader_.reset();
    selected_item_.reset();
    selected_item_previous_shader_.reset();
}

void Player::draw(std::string current_user, const MainCamera& camera) const {
    if ((camera.get_mode() != CAMERA_CUSTOM || username_ != current_user) && online_) {
        for (const auto& object : model_)
            object->draw_offset(get_position().x, get_position().y, get_position().z);
    }
    if (selected_item_ != nullptr && online_) {
        selected_item_->draw_offset(get_position().x, get_position().y + 2.0f, get_position().z);
    }
}

bool Player::move(MainCamera& camera, const std::vector<bool>& keybinds, float dt) {
    assert(keybinds.size() >= 4);
    if (camera.get_mode() == CAMERA_FREE || !online_ || (!keybinds[0] && !keybinds[1] && !keybinds[2] && !keybinds[3])) {
        return false;
    }
    Vector3 direction = camera.get_direction();
    Vector3 left = Vector3{direction.z, 0.0f, -direction.x};
    float dx = (keybinds[0]*direction.x-keybinds[2]*direction.x-keybinds[3]*left.x+keybinds[1]*left.x);
    float dz = (keybinds[0]*direction.z-keybinds[2]*direction.z-keybinds[3]*left.z+keybinds[1]*left.z);
    float magnitude = sqrt(pow(dx,2.0) + pow(dz,2.0));
    if (magnitude == 0) return false;
    dx = dx/magnitude*dt*speed_;
    dz = dz/magnitude*dt*speed_;

    hitbox_.set_position(Vector3{hitbox_.get_position().x + dx, hitbox_.get_position().y, hitbox_.get_position().z + dz});
    return true;
}

uint32_t Player::try_pickup(MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds) const {
    uint32_t id = 0;
    if (keybinds[9]) {
        Ray ray = Ray{camera.get_position(), camera.get_direction()};
        float min_distance = std::numeric_limits<float>::infinity();
        for (const auto& p : world->get_objects()) {
            std::shared_ptr<Item> item = std::dynamic_pointer_cast<Item>(p.second);
            if (item == nullptr) {
                continue;
            }
            RayCollision c = GetRayCollisionBox(ray, p.second->get_bounding_box());
            if (c.hit) {
                float d = c.distance;
                if (d < min_distance && d <= pickup_range_) {
                    id = p.first;
                    min_distance = d;
                }
            }
        }
    }
    return id;
}

void Player::set_position(Vector3 position) {
    hitbox_.set_position(Vector3{position.x, position.y+0.5f, position.z});
}

void Player::add_to_model(std::unique_ptr<Object3d>&& object) {
    model_.push_back(std::move(object));
}

void Player::set_shader(std::shared_ptr<Shader> shader) {
    shader_ = shader;
    for (auto& object : model_) {
        object->set_shader(shader_);
    }
    if (selected_item_ != nullptr) {
        selected_item_->set_shader(shader_);
    }
}

std::shared_ptr<Shader> Player::get_shader() const {
    return shader_;
}

void Player::update(std::map<std::string, std::shared_ptr<Event>>& event_buffer, MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    bool moved = move(camera, keybinds, dt);
    uint32_t pickup_id = try_pickup(camera, world, keybinds);
    use_item(event_buffer, camera, world, keybinds, dt);
    if (moved) {
        event_buffer["PlayerMoveEvent"] = std::make_shared<PlayerMoveEvent>(shared_from_this());
    }
    if (pickup_id != 0) {
        std::shared_ptr<Item> dropped = drop_item(event_buffer,camera,world,keybinds,dt);
        if (dropped != nullptr) {
            uint32_t id = world->load_object(dropped, dropped->get_shader());
            event_buffer["ItemDropEvent"] = std::make_shared<ItemDropEvent>(shared_from_this());
            if (event_buffer.find("ObjectLoadEvent") == event_buffer.end()) {
                std::shared_ptr<ObjectLoadEvent> load_event = std::make_shared<ObjectLoadEvent>(std::map<uint32_t,std::shared_ptr<Object3d>>{}, get_username());
                load_event->add(id, dropped);
                event_buffer["ObjectLoadEvent"] = load_event;
            } else {
                std::dynamic_pointer_cast<ObjectLoadEvent>(event_buffer["ObjectLoadEvent"])->add(id, dropped);
            }
        }
        std::shared_ptr<Item> item = std::dynamic_pointer_cast<Item>(world->get_objects().at(pickup_id));
        set_item(item);
        world->remove_object(pickup_id);
        event_buffer["ItemPickupEvent"] = std::make_shared<ItemPickupEvent>(item, get_username());
        if (event_buffer.find("ObjectRemoveEvent") == event_buffer.end()) {
            std::shared_ptr<ObjectRemoveEvent> remove_event = std::make_shared<ObjectRemoveEvent>(std::vector<uint32_t>{}, get_username());
            remove_event->add(pickup_id);
            event_buffer["ObjectRemoveEvent"] = std::move(remove_event);
        } else {
            std::dynamic_pointer_cast<ObjectRemoveEvent>(event_buffer["ObjectRemoveEvent"])->add(pickup_id);
        }
    } else if (keybinds[9]) {
        std::shared_ptr<Item> dropped = drop_item(event_buffer,camera,world,keybinds,dt);
        if (dropped == nullptr)
            return;
        event_buffer["ItemDropEvent"] = std::make_shared<ItemDropEvent>(shared_from_this());
        uint32_t id = world->load_object(dropped, dropped->get_shader());
        if (event_buffer.find("ObjectLoadEvent") == event_buffer.end()) {
            std::shared_ptr<ObjectLoadEvent> load_event = std::make_shared<ObjectLoadEvent>(std::map<uint32_t,std::shared_ptr<Object3d>>{}, get_username());
            load_event->add(id, dropped);
            event_buffer["ObjectLoadEvent"] = load_event;
        } else {
            std::dynamic_pointer_cast<ObjectLoadEvent>(event_buffer["ObjectLoadEvent"])->add(id, dropped);
        }
    }
}

void Player::set_item(std::shared_ptr<Item> item) {
    if (selected_item_ != nullptr && item.get() == selected_item_.get())
        return;
    selected_item_previous_shader_ = item->get_shader();
    selected_item_ = item;
    selected_item_->set_position(Vector3{0.0f,0.0f,0.0f});
}

std::shared_ptr<Item> Player::drop_item(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    if (selected_item_ == nullptr)
        return nullptr;
    std::shared_ptr<Item> item = selected_item_;
    selected_item_ = nullptr;
    item->prepare_drop(event_buffer,camera,shared_from_this(),world,keybinds,dt);
    item->set_position(get_position());
    item->set_shader(selected_item_previous_shader_ == nullptr ? shader_ : selected_item_previous_shader_);
    return item;
}
void Player::use_item(std::map<std::string, std::shared_ptr<Event>>& event_buffer, const MainCamera& camera, std::shared_ptr<World> world, const std::vector<bool>& keybinds, float dt) {
    if (selected_item_ == nullptr)
        return;
    selected_item_->use(event_buffer, camera, shared_from_this(), world, keybinds, dt);
}

void Player::on_join() {
    online_ = true;
}

void Player::on_disconnect() {
    online_ = false;
}

bool Player::is_online() const {
    return online_;
}

const Cube& Player::get_hitbox() {
    return hitbox_;
}

std::string Player::get_username() {
    return username_;
}

Vector3 Player::get_position() const {
    return Vector3{hitbox_.get_position().x, hitbox_.get_position().y - 0.5f, hitbox_.get_position().z};
}

std::string Player::to_string() const {
    std::string item_string = (selected_item_ == nullptr) ? "null_item" : selected_item_->to_string();
    std::string result = "Player " + 
        username_ + " " +
        std::to_string(get_position().x) + " " + std::to_string(get_position().y) + " " + std::to_string(get_position().z) + " "
        + std::to_string((int)online_) + " (" + item_string + ")(";
    for (const auto& object : model_) {
        result += "(" + object->to_string() + ")";
    }
    result += ")";
    return result;
}