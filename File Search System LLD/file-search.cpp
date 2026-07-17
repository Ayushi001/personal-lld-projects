/******************************************************************************
Unix File Search System: Alex Xu Chp 6
Given a file and directory setup, return the list of files that match the search
criteria given by user
eg: (owner=ayushi && (createdAt<yesterday || size>10)) query should return all
files matching the criteria in a directory
*******************************************************************************/

/*
enum Attributes{
    OWNER,
    SIZE,
    CREATED_AT
}
ENTITIES
File: owner,isFile (false for folder), createdAt, size, vector<File*> children (sub-folders/file if folder)
 - addFile(File* file)
 - removeFile()
 - string getPath(File* root) : returns the full directory path traversing from root to this file node (SKIP for now)

FileSearch:
  - vector<File*> searchFiles(File* file, Criteria criteria): returns list of files matching the file search criteria

Criteria: Predicate p
   - bool isMatch(File* file)

Interface Predicate:
- bool isMatch(File* file) : returns true if the ComparisonOperator constructed for this predicate returns true for target vals

class SimplePredicate: public Predicate
- ComparisonOperator op
- bool isMatch(File*file): returns op.isMatch(file)

// composite predicates
class AndPredicate: public Predicate{
    vector<Predicate*> predicates
    - bool isMatch(File* file) returns true if ALL of the predicate's isMatch return true
}
class OrPredicate: public Predicate{
    vector<Predicate*> predicates
    - bool isMatch(File* file) returns true if EITHER of the predicate's isMatch return true
}
class NotPredicate: public Predicate{
    Predicate* predicate // since NOT is a unary operator
    - bool isMatch(File* file) returns ! of predicate's isMatch() value
}

Interface ComparisonOperator:
    - bool isMatch(File * file)
class EqualOperator: public ComparisonOperator
    - targetAttribute, targetValue
    - bool isMatch(File* file) : returns true if file's targetAttribute value is same as targetValue
class GreaterThanOperator: public ComparisonOperator
   - targetAttribute, targetValue
   - bool isMatch(File* file) : returns true if file's targetAttribute value is greater as targetValue
class LesserThanOperator: public ComparisonOperator
   - targetAttribute, targetValue
   - bool isMatch(File* file) : returns true if file's targetAttribute value is lesser as targetValue

 FileManager: singleton facade class which manages files and performs file search
*/
#include <bits/stdc++.h>
using namespace std;
/*
File: owner,isFile (false for folder), createdAt, size, vector<File*> children (sub-folders/file if folder)
 - addFile(File* file)
 - removeFile()
 - string getPath(File* root) : returns the full directory path traversing from root to this file node (SKIP for now)
*/
class File
{
    string owner;
    int size, createdAt;
    bool isFile;
    string fileName;
    vector<File *> subFiles;

public:
    File(string owner, int size, int createdAt, bool isFile, string name) : owner(owner), size(size),
                                                                            createdAt(createdAt), isFile(isFile), fileName(name)
    {
    }
    void addFile(File *file)
    {
        subFiles.push_back(file);
    }
    string getOwner()
    {
        return owner;
    }
    int getSize()
    {
        return size;
    }
    int getCreationTime()
    {
        return createdAt;
    }
    bool getIsFile()
    {
        return isFile;
    }
    vector<File *> getSubFiles()
    {
        return subFiles;
    }
    string getName()
    {
        return fileName;
    }
};

/*
Interface ComparisonOperator:
    - bool isMatch(File * file)
class EqualOperator: public ComparisonOperator
    - targetAttribute, targetValue
    - bool isMatch(File* file) : returns true if file's targetAttribute value is same as targetValue
class GreaterThanOperator: public ComparisonOperator
   - targetAttribute, targetValue
   - bool isMatch(File* file) : returns true if file's targetAttribute value is greater as targetValue
class LesserThanOperator: public ComparisonOperator
   - targetAttribute, targetValue
   - bool isMatch(File* file) : returns true if file's targetAttribute value is lesser as targetValue
*/
enum Attribute
{
    OWNER,     // string
    SIZE,      // int
    CREATED_AT // int
};
class ComparisonOperator
{
public:
    virtual bool isMatch(File *file) = 0;
};
class EqualOperator : public ComparisonOperator
{
    Attribute targetAttribute;
    string targetValue;

public:
    EqualOperator(Attribute targetAttribute, string targetValue) : targetAttribute(targetAttribute), targetValue(targetValue)
    {
    }
    bool isMatch(File *file) override
    {
        switch (targetAttribute)
        {
        case (Attribute::OWNER):
        {
            return file->getOwner() == targetValue;
        }
        case (Attribute::SIZE):
        {
            return file->getSize() == stoi(targetValue);
        }
        case (Attribute::CREATED_AT):
        {
            return file->getCreationTime() == stoi(targetValue);
        }
        default:
            return false;
        }
    }
};
class GreaterThanOperator : public ComparisonOperator
{
    Attribute targetAttribute;
    string targetValue;

public:
    GreaterThanOperator(Attribute targetAttribute, string targetValue) : targetAttribute(targetAttribute), targetValue(targetValue)
    {
    }
    bool isMatch(File *file) override
    {
        switch (targetAttribute)
        {
        case (Attribute::OWNER):
        {
            return file->getOwner() > targetValue;
        }
        case (Attribute::SIZE):
        {
            return file->getSize() > stoi(targetValue);
        }
        case (Attribute::CREATED_AT):
        {
            return file->getCreationTime() > stoi(targetValue);
        }
        default:
            return false;
        }
    }
};
class LesserThanOperator : public ComparisonOperator
{
    Attribute targetAttribute;
    string targetValue;

public:
    LesserThanOperator(Attribute targetAttribute, string targetValue) : targetAttribute(targetAttribute), targetValue(targetValue)
    {
    }
    bool isMatch(File *file) override
    {
        switch (targetAttribute)
        {
        case (Attribute::OWNER):
        {
            return file->getOwner() > targetValue;
        }
        case (Attribute::SIZE):
        {
            return file->getSize() > stoi(targetValue);
        }
        case (Attribute::CREATED_AT):
        {
            return file->getCreationTime() > stoi(targetValue);
        }
        default:
            return false;
        }
    }
};

/*
Interface Predicate:
- bool isMatch(File* file) : returns true if the ComparisonOperator constructed for this predicate returns true for target vals

class SimplePredicate: public Predicate
- ComparisonOperator op
- bool isMatch(File*file): returns op.isMatch(file)

// composite predicates
class AndPredicate: public Predicate{
    vector<Predicate*> predicates
    - bool isMatch(File* file) returns true if ALL of the predicate's isMatch return true
}
class OrPredicate: public Predicate{
    vector<Predicate*> predicates
    - bool isMatch(File* file) returns true if EITHER of the predicate's isMatch return true
}
class NotPredicate: public Predicate{
    Predicate* predicate // since NOT is a unary operator
    - bool isMatch(File* file) returns ! of predicate's isMatch() value
}
*/
class Predicate
{
public:
    virtual bool isMatch(File *file) = 0;
};
class SimplePredicate : public Predicate
{
    ComparisonOperator *op;

public:
    SimplePredicate(ComparisonOperator *op) : op(op)
    {
    }
    bool isMatch(File *file) override
    {
        return op->isMatch(file);
    }
};
// composite predicates (COMPOSITE DESIGN PATTERN)
class AndPredicate : public Predicate
{
    vector<Predicate *> predicates;
    // can further contain simple/composite predicates - heirarchal structure
public:
    AndPredicate(vector<Predicate *> predicates) : predicates(predicates)
    {
    }
    bool isMatch(File *file) override
    {
        bool isMatch = true;
        for (auto predicate : predicates)
        {
            isMatch &= predicate->isMatch(file);
        }
        return isMatch;
    }
};
class OrPredicate : public Predicate
{
    vector<Predicate *> predicates;

public:
    OrPredicate(vector<Predicate *> predicates) : predicates(predicates)
    {
    }
    bool isMatch(File *file) override
    {
        bool isMatch = false;
        for (auto predicate : predicates)
        {
            isMatch |= predicate->isMatch(file);
        }
        return isMatch;
    }
};
class NotPredicate : public Predicate
{
    Predicate *predicate; // since unary operator
public:
    NotPredicate(Predicate *predicate) : predicate(predicate)
    {
    }
    bool isMatch(File *file) override
    {
        return !predicate->isMatch(file);
    }
};

/*
FileSearch:
  - vector<File*> searchFiles(File* file, Criteria criteria): returns list of files matching the file search criteria

Criteria: Predicate p
   - bool isMatch(File* file)
*/
// does a file search based on a predicate from a root directory
class FileSearch
{
    Predicate *predicate;

public:
    FileSearch(Predicate *predicate) : predicate(predicate)
    {
    }
    vector<File *> searchFiles(File *root)
    {
        // do a BFS over all children and add the file/leaf nodes which match the predicate
        vector<File *> result;
        queue<File *> q;
        q.push(root);
        while (!q.empty())
        {
            File *file = q.front();
            q.pop();
            if (file->getIsFile() && predicate->isMatch(file))
            {
                result.push_back(file);
            }
            else
            {
                for (auto subFile : file->getSubFiles())
                {
                    q.push(subFile);
                }
            }
        }
        return result;
    }
};

int main()
{
    //(owner==ayushi && (createdAt<10 || size>50))
    Predicate *searchPredicate = new AndPredicate({new SimplePredicate(new EqualOperator(Attribute::OWNER, "ayushi")),
                                                   new OrPredicate({
                                                    new SimplePredicate(new LesserThanOperator(Attribute::CREATED_AT, "10")),
                                                    new SimplePredicate(new GreaterThanOperator(Attribute::SIZE, "50"))
                                                })
                                            });

    // File(string owner, int size, int createdAt, bool isFile,string name)
    File *root = new File("owner1", 70, 9, false, "root");
    File *file1 = new File("ayushi", 10, 10, true, "file1");
    File *file2 = new File("ayushi", 100, 8, true, "file2");
    root->addFile(file1);
    root->addFile(file2);

    FileSearch *searchEngine = new FileSearch(searchPredicate);
    vector<File *> res = searchEngine->searchFiles(root);
    for (auto file : res)
    {
        cout << file->getName() << '\n';
    }
    return 0;
}
/*
OUTPUT:
file2
*/