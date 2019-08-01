#include "QuakeMessage.hpp"

QuakeMessage::QuakeMessage() { this->length = 0; }

QuakeMessage::QuakeMessage(unsigned int len) { this->length = len; }

QuakeMessage::QuakeMessage(QuakeMessage const &mes) {
  this->length = mes.length;
  for (int i = 0; i < this->length; i++)
    this->mes[i] = mes.mes[i];
}

QuakeMessage &QuakeMessage::operator=(QuakeMessage const &mes) {
  this->length = mes.length;
  for (int i = 0; i < this->length; i++)
    this->mes[i] = mes.mes[i];
  return *(this);
}

char &QuakeMessage::operator[](int i) { return this->mes[i]; }

char QuakeMessage::operator[](int i) const { return this->mes[i]; }

void QuakeMessage::copy_message(char *c) const {
  for (int i = 0; i < this->length; i++)
    c[i] = this->mes[i];
}
