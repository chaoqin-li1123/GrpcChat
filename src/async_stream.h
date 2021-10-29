
#ifndef ASYNC_STREAM_H
#define ASYNC_STREAM_H

enum class Action { INIT, READ, WRITE, FINISH };

template <typename Actor>
struct ActionTag {
  ActionTag(Actor* actor, Action action) : actor_(actor), action_(action) {}
  void act(bool ok) {
    switch (action_) {
      case Action::INIT:
        actor_->onInitComplete(ok);
        break;
      case Action::READ:
        actor_->onReadComplete(ok);
        break;
      case Action::WRITE:
        actor_->onWriteComplete(ok);
        break;
      case Action::FINISH:
        actor_->onFinishComplete(ok);
        break;
    }
  }
  Actor* actor_;
  Action action_;
};

struct AsyncStream {
  virtual void onReadComplete(bool ok) = 0;

  virtual void onInitComplete(bool ok) = 0;

  virtual void onWriteComplete(bool ok) = 0;

  virtual void onFinishComplete(bool ok) = 0;
};

#endif