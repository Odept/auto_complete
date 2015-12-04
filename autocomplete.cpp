#include <iostream>
#include <vector>
#include <fstream>

// ============================================================================
// Ternary Search Trie
struct Node
{
	Node(Node* f_pMiddle):
		l(NULL), m(f_pMiddle), r(NULL),
		nRecords(f_pMiddle->nRecords)
	{}

	Node* l;
	Node* m;
	Node* r;

	unsigned nRecords;

	virtual bool hasKey() const { return false; }

protected:
	explicit Node(): l(NULL), m(NULL), r(NULL), nRecords(1) {}
};

static const char NoChar = 0xFF;
// "Radix" Node
struct RNode : public Node
{
	RNode(const char f_c, Node* f_pMiddle):
		Node(f_pMiddle),
		C(f_c)
	{}
	char C;

protected:
	RNode(): C(NoChar) {}
};

// KeyNode inherits RNode to handle prefix cases
struct KeyNode : public RNode
{
	KeyNode(const std::string& f_str): Str(f_str) {}
	const std::string Str;

	virtual bool hasKey() const { return true; }
};


class CTST
{
public:
	typedef std::vector<std::string> strings_t;
public:
	CTST(unsigned f_dim, unsigned f_uFirst): m_heads(f_dim), m_uFirst(f_uFirst), m_nCount(0) {}

	unsigned getCount() const { return m_nCount; }

	bool insert(const std::string& f_str)
	{
		if(f_str.empty())
			return true;

		const char c = f_str[0];
		if(unsigned(c - m_uFirst) >= m_heads.size())
			return false;

		bool bOk = insert(m_heads[c - m_uFirst], f_str, 0);
		if(bOk)
			m_nCount++;
		return bOk;
	}

	strings_t beginWith(const std::string& f_str) const
	{
		strings_t strings;
		if(!f_str.empty())
		{
			const char c = f_str[0];
			if(unsigned(c - m_uFirst) < m_heads.size())
			{
				const Node* p = search(m_heads[c - m_uFirst], f_str, 0);
				pre_order(p, strings);
			}
		}
		return strings;
	}

private:
	bool insert(Node*& f_rpNode, const std::string& f_str, unsigned f_i)
	{
		// Insert new record
		if(!f_rpNode)
		{
			// Create an str-char-node if currently at the last symbol
			// Such nodes can't be split
			f_rpNode = new KeyNode(f_str);
			return true;
		}

		// Handle str-nodes (compare and split)
		if(f_rpNode->hasKey())
		{
			KeyNode* pKeyNode = dynamic_cast<KeyNode*>(f_rpNode);
			const std::string& strNode = pKeyNode->Str;

			// If already exists
			if(f_str == strNode)
				return false;
			// If the code reaches here, it means that the current key node is a prefix of the node being inserted

			if(f_i < strNode.length())
			{
				// Per-char split: add a char node with the current leaf in the middle
				RNode* p = new RNode(strNode[f_i], pKeyNode);
				f_rpNode = p;
			}
			else if(pKeyNode->C == NoChar)
			{
				// The key node can't be split (the parent radix-node is the last symbol of the key),
				// but there's no symbol associated with the node, so do it
				pKeyNode->C = f_str[f_i];
			}
		}

		// Proceed to children
		bool bOk;

		const char c = f_str[f_i];
		const char node_c = dynamic_cast<RNode*>(f_rpNode)->C;
		if(c < node_c)
			bOk = insert(f_rpNode->l, f_str, f_i);
		else if(c == node_c)
			bOk = insert(f_rpNode->m, f_str, f_i + 1);
		else
			bOk = insert(f_rpNode->r, f_str, f_i);

		if(bOk)
			f_rpNode->nRecords++;

		return bOk;
	}

	const Node* search(const Node* f_pNode, const std::string& f_str, unsigned f_i) const
	{
		/*std::cout << std::endl << f_str << '[' << f_i << "]: ";
		if(f_pNode)
		{
			const RNode* p = dynamic_cast<const RNode*>(f_pNode);
			if(p->C == NoChar)
				std::cout << '"' << dynamic_cast<const KeyNode*>(p)->Str << '"';
			else
				std::cout << p->C;
			std::cout << " (" << p->l << ", " << p->m << ", " << p->r << ")";
		}
		else
			std::cout << "NULL";
		std::cout << std::endl;//*/
		if(!f_pNode || f_i >= f_str.length())
			return f_pNode;

		const char c = f_str[f_i];
		const char node_c = dynamic_cast<const RNode*>(f_pNode)->C;
		// Skip NoChar (pure str-nodes) checks, because such nodes have null-links only
		if(c < node_c)
			return search(f_pNode->l, f_str, f_i);
		else if(c == node_c)
			return search(f_pNode->m, f_str, f_i + 1);
		else
			return search(f_pNode->r, f_str, f_i);
	}

	void pre_order(const Node* f_pNode, strings_t& f_strings) const
	{
		if(!f_pNode)
			return;

		if(f_pNode->hasKey())
			f_strings.push_back(dynamic_cast<const KeyNode*>(f_pNode)->Str);
		pre_order(f_pNode->l, f_strings);
		pre_order(f_pNode->m, f_strings);
		pre_order(f_pNode->r, f_strings);

	}

private:
	std::vector<Node*> m_heads;
	unsigned m_uFirst;
	unsigned m_nCount;
};

// ============================================================================
#define MSG(m) std::cout << m << std::endl


#include <termios.h>	//termios, TCSANOW, ECHO, ICANON
#include <unistd.h>		//STDIN_FILENO
static char getch()
{
	static termios told, tnew;

	// tcgetattr gets the parameters of the current terminal.
	// STDIN_FILENO will tell tcgetattr that it should write the settings
	// of stdin to oldt
	fflush(stdin);
	tcgetattr(STDIN_FILENO, &told);

	// ICANON takes care that one line at a time will be processed
	// (return if it sees a "\n" or an EOF or an EOL)
	tnew = told;
	tnew.c_lflag &= ~(ICANON | ECHO);		

	// TCSANOW makes attributes change immediately
	tcsetattr(STDIN_FILENO, TCSANOW, &tnew);
	char x = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &told);

	return x;
}


int main(int, char**)
{
	static const char* s_path = "words.txt";
	std::ifstream fin(s_path);
	if(!fin.is_open())
	{
		MSG("Failed to open the input file \"" << s_path << "\"");
		return 1;
	}

	CTST words('z' - 'a' + 1, 'a');

	unsigned nRead = 0;
	unsigned nSkipped = 0;
	for(std::string s;;)
	{
		fin >> s;
		if(fin.eof())
			break;
		nRead++;

		//MSG("Insert \"" << s << "\"");
		if( !words.insert(s) )
		{
		//	MSG("  already exists");
			nSkipped++;
		}
	}
	MSG(nRead << " words read: " << words.getCount() << " inserted, " << nSkipped << " skipped");

	// Statistics
	for(char c = 'a'; c <= 'z'; c++)
	{
		char str[] = {c, 0};
		MSG(c << ": " << words.beginWith(str).size());
	}

	// Autocomplete loop
	#define KEY_TAB			9
	#define KEY_ESCAPE		27
	#define KEY_BACKSPACE	127
	static const char* s_prompt = "> ";

	std::cout << "Input a prefix string. Double <TAB> to auto-complete, <ESC> to exit." << std::endl;
	std::cout << s_prompt;
	for(std::string str;;)
	{
		static bool bTab = false;

		char c = getch();
		if(c == KEY_ESCAPE)
		{
			std::cout << std::endl;
			break;
		}

		if(c == KEY_TAB)
		{
			if(bTab)
			{
				CTST::strings_t strings = words.beginWith(str);
				if(unsigned n = strings.size())
				{
					std::cout << std::endl;
					for(unsigned i = 0; i < n; i++)
						std::cout << strings[i] << ' ';
				}
				std::cout << std::endl << s_prompt << str;
			}
			else if(!str.empty())
				bTab = true;
		}
		else
		{
			bTab = false;
			if(c == KEY_BACKSPACE)
			{
				if(unsigned len = str.length())
				{
					str.resize(len - 1);
					std::cout << '\b' << ' ' << '\b';
				}
			}
			else if(c >= 'a' && c <= 'z')
			{
				str.append(1, c);
				std::cout << c;
			}
		}
	}

	return 0;
}

