#pragma once

#include "Defines.h"

namespace Frumpy
{
	template<typename T>
	class FRUMPY_API List
	{
	public:
		class Node;
		
		List()
		{
			this->head = nullptr;
			this->tail = nullptr;
			this->count = 0;
		}

		List(const List& list)
		{
			this->head = nullptr;
			this->tail = nullptr;
			this->count = 0;

			for (const Node* node = list.head; node; node = node->next)
				this->AddTail(node->value);
		}

		virtual ~List()
		{
		}

		Node* GetHead() { return this->head; }
		Node* GetTail() { return this->tail; }

		const Node* GetHead() const { return this->head; }
		const Node* GetTail() const { return this->tail; }

		void AddHead(T value)
		{
			if (this->head)
				this->InsertBefore(this->head, value);
			else
			{
				this->head = this->tail = new Node(value, this);
				this->count = 1;
			}
		}

		void AddTail(T value)
		{
			if (this->tail)
				this->InsertAfter(this->tail, value);
			else
			{
				this->head = this->tail = new Node(value, this);
				this->count = 1;
			}
		}

		void InsertAfter(Node* node, T value)
		{
			Node* newNode = new Node(value, this);
			newNode->Couple(node, node->next);
			if (!newNode->next)
				this->tail = newNode;
			this->count++;
		}

		void InsertBefore(Node* node, T value)
		{
			Node* newNode = new Node(value, this);
			newNode->Couple(node->prev, node);
			if (!newNode->prev)
				this->head = newNode;
			this->count++;
		}

		void Remove(Node* node)
		{
			if (node->list == this)
			{
				if (node == this->head)
					this->head = this->head->next;
				if (node == this->tail)
					this->tail = this->tail->prev;
				node->Decouple();
				delete node;
				this->count--;
			}
		}

		void Clear()
		{
			while (this->count > 0)
				this->Remove(this->head);
		}

		void Delete()
		{
			while (this->count > 0)
			{
				delete this->head->value;
				this->Remove(this->head);
			}
		}

		unsigned int GetCount() const
		{
			return this->count;
		}

		class Node
		{
			friend class List;

		public:
			Node(T value, List<T>* list)
			{
				this->value = value;
				this->list = list;
			}

			virtual ~Node()
			{
			}

			Node* GetNext() { return this->next; }
			Node* GetPrev() { return this->prev; }

			const Node* GetNext() const { return this->next; }
			const Node* GetPrev() const { return this->prev; }

			T value;

		private:
			void Decouple()
			{
				if (this->prev)
					this->prev->next = this->next;
				if (this->next)
					this->next->prev = this->prev;
				this->next = nullptr;
				this->prev = nullptr;
			}

			void Couple(Node* before, Node* after)
			{
				this->prev = before;
				this->next = after;
				if (before)
					before->next = this;
				if (after)
					after->prev = this;
			}

			List<T>* list;
			Node* next;
			Node* prev;
		};

	private:

		Node* head;
		Node* tail;
		int count;
	};
}