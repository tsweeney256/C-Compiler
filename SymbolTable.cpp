#include <string>
#include <vector>
#include <iostream>
#include "SymbolTable.hpp"

class SymbolTable::Impl
{
public:
	Impl(int size);
	~Impl();
	bool add(const std::string& name, int type, std::vector<int>* signature);
	int peek(const std::string& name) const;
	const std::vector<int>* getSignature(const std::string& name) const;

private:
	struct SymbolTableData
	{
		SymbolTableData(const std::string& name, int type, std::vector<int>* signature);
		~SymbolTableData();
		std::string m_name;
		int m_type;
		std::vector<int>* m_signature;
		SymbolTableData* m_next;
	};
	SymbolTableData** m_symTab;
	size_t m_size;
	size_t m_filled;

	int hash(const std::string& name);
	void growTable();
};

SymbolTable::Impl::SymbolTableData::SymbolTableData(const std::string& name, int type, std::vector<int>* signature)
	: m_name(name), m_type(type), m_signature(signature), m_next(NULL) {}

SymbolTable::Impl::SymbolTableData::~SymbolTableData()
{
	delete m_signature;
	delete m_next;
}

SymbolTable::Impl::Impl(int size)
{
	m_size = size;
	m_symTab = new SymbolTableData*[m_size];
	m_filled = 0;
	for(size_t i=0; i<m_size; ++i){
		m_symTab[i] = NULL;
	}
}

SymbolTable::Impl::~Impl()
{
	for(size_t i=0; i<m_size; ++i){
		delete m_symTab[i];
	}
	delete[] m_symTab;
}

int SymbolTable::Impl::hash(const std::string& name)
{
	//djb2 algorithm
	//http://www.cse.yorku.ca/~oz/hash.html
	unsigned int hash = 5381;
	for(std::string::const_iterator it = name.begin(); it != name.end(); ++it){
		hash = ((hash << 5) + hash) + *it;
	}
	return hash % m_size;
}

void SymbolTable::Impl::growTable()
{
	size_t oldSize = m_size;
	m_size *= 2;
	m_filled = 0;
	SymbolTableData** oldSymTab = m_symTab;
	m_symTab = new SymbolTableData*[m_size];

	for(size_t i=0; i<oldSize; ++i){
		if(!oldSymTab[i]){
			continue;
		}
		SymbolTableData* currSym = oldSymTab[i];
		add(currSym->m_name, currSym->m_type, currSym->m_signature);
		while((currSym = currSym->m_next)){
			add(currSym->m_name, currSym->m_type, currSym->m_signature);
		}
		delete oldSymTab[i];
	}
	delete[] oldSymTab;
}

bool SymbolTable::Impl::add(const std::string& name, int type, std::vector<int>* signature)
{
	if(m_filled >= m_size*2){
		growTable();
	}

	int key = hash(name);
	if(!m_symTab[key]){
		m_symTab[key] = new SymbolTableData(name, type, signature);
	}
	else{
		SymbolTableData* currSym = m_symTab[key];
		if(!currSym->m_name.compare(name)){
			delete signature;
			return false;
		}
		while(currSym->m_next){
			currSym = currSym->m_next;
			if(!currSym->m_name.compare(name)){
				delete signature;
				return false;
			}
		}
		currSym->m_next = new SymbolTableData(name, type, signature);
	}
	m_filled++;
	return true;
}

int SymbolTable::Impl::peek(const std::string& name) const
{
	for(size_t i=0; i<m_size; ++i){
		if(!m_symTab[i]){
			continue;
		}

		SymbolTableData* currSym = m_symTab[i];
		if(!name.compare(currSym->m_name)){
			return currSym->m_type;
		}
		while((currSym = currSym->m_next)){
			if(!name.compare(currSym->m_name)){
				return currSym->m_type;
			}
		}
	}
	return NOT_FOUND;
}

const std::vector<int>* SymbolTable::Impl::getSignature(const std::string& name) const
{
	for(size_t i=0; i<m_size; ++i){
		if(!m_symTab[i]){
			continue;
		}

		SymbolTableData* currSym = m_symTab[i];
		if(!name.compare(currSym->m_name)){
			return currSym->m_signature;
		}
		while((currSym = currSym->m_next)){
			if(!name.compare(currSym->m_name)){
				return currSym->m_signature;
			}
		}
	}
	return NULL;
}

SymbolTable::SymbolTable(int size) : pimpl(new Impl(size)) {}

SymbolTable::~SymbolTable()
{
	delete pimpl;
}

bool SymbolTable::add(const std::string& name, int type, std::vector<int>* signature)
{
	return pimpl->add(name, type, signature);
}

int SymbolTable::peek(const std::string& name) const
{
	return pimpl->peek(name);
}

const std::vector<int>* SymbolTable::getSignature(const std::string& name) const
{
	return pimpl->getSignature(name);
}
