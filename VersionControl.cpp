// Create a version control system inspired by Git implementing the data structures - trees, heaps, hashmap from scratch
// Updates -> Merging done, all classes completed except heap
// Next -> Add whatever is needed in heap, and put heap instead of vector for system wide analytics.
// Also make sure that the heap is updated everytime a file is updated

#include<iostream>
#include<ctime>
#include<vector>
#include<algorithm>
using namespace std;

// Constants
int SIZE = 1e2;

class TreeNode{
public:
    int version_id;
    string content;
    string message;
    time_t created_timestamp;
    time_t snapshot_timestamp;
    time_t last_modified;
    TreeNode* Parent;
    vector<TreeNode*> children;
    bool snapshot_taken;

    TreeNode(int v_id, string c, string m) {
        version_id = v_id;
        content = c;
        message = m;
        created_timestamp = time(nullptr);
        Parent = nullptr;
        snapshot_timestamp;
        snapshot_taken = false;
        last_modified = created_timestamp;
    }

    void snapshot(string m) {
        snapshot_timestamp = time(nullptr);
        message = m;
        snapshot_taken = true;
    }
};

#include <iostream>
#include <vector>
using namespace std;

class MaxHeap {
    vector<int> heap;
    vector<Tree*> v;

    void heapifyup(int idx) {
        while ((idx>0) && (heap[(idx-1)/2]<heap[idx])) {
            swap(heap[idx], heap[(idx - 1) / 2]);
            swap(v[idx], v[(idx - 1) / 2]);
            idx=(idx-1)/2;
        }
    }

    void heapifydown(int idx) {
        int n = heap.size();
        while (true) {
            int l = 2*idx+1, r=2*idx+2, c=idx;
            if ((l<n) && (heap[l]>heap[c])) c=l;
            if ((r<n) && (heap[r]>heap[c])) c=r;
            if (c == idx) break;
            swap(heap[idx], heap[c]);
            swap(v[idx], v[c]);
            idx=c;
        }
    }

public:
    void insert(int val, Tree* file) {
        heap.push_back(val);
        v.push_back(file);
        heapifyup(heap.size()-1);
    }

    int getMax() {
        if (heap.empty()) cout<<"Empty!!"<<'\n';
        return heap[0];
    }

    Tree* removeTop() {
        if (heap.empty()) cout<<"Empty!!"<<'\n';
        Tree* maxVal = v[0];
        v[0] = v.back();
        heap[0] = heap.back();
        heap.pop_back();
        v.pop_back();
        v.pop_back();
        if (!heap.empty()) heapifydown(0);
        return maxVal;
    }
};

template <typename T1, typename T2>
class HashMap {

private:
    int hash_fn(int key) {
        return key%SIZE;
    }

    int hash_fn(string key) {
        int x=0;
        for (char c:key) x+= (c-'\0');
        return x%SIZE;
    }

public:
    vector<T1> keys;
    vector<T2> nodes;

    HashMap() {
        this->keys = vector<int>(SIZE, -1);
        this->nodes = vector<T2>(SIZE, nullptr);
    }



    void insert(T1 key, T2 val) {
        int hashed = hash_fn(key);
        if (keys[hashed] == -1) {
            keys[hashed] = key;
            nodes[hashed] = val;
        } else {
            // Full round
            int tph = hashed;
            tph = (tph+1)%SIZE;
            while ((tph!=hashed) && keys[tph]!=-1) {
                tph = (tph+1)%SIZE;
            }
            if (tph==hashed) cout<<"HASHMAP FULL"<<'\n';
            else {
                keys[tph] = key;
                nodes[tph] = val;
            }
        }
    }

    T2 get(T1 key) {
        int hashed = hash_fn(key);
        while (keys[hashed]!=key) hashed = (hashed+1)%SIZE;
        if (keys[hashed]==key) return nodes[hashed];
        else return nullptr;
    }

};

class Tree {
public:
    TreeNode* root;
    TreeNode* Active;
    HashMap<int, TreeNode*> version_ctrl;
    int total_versions;
    string file_name;
    vector<pair<time_t, int>> snapshotted_versions;
    time_t last_modified;

    Tree (string filename) {
        file_name = filename;
        TreeNode f(0, "", "created file");
        TreeNode* file = &f;
        root = file;
        Active = file;
        file->snapshot("Initial Snapshot");
        snapshotted_versions.push_back({file->snapshot_timestamp, file->version_id});
        version_ctrl.insert(file->version_id, file);
        total_versions = 1;
        last_modified = file->snapshot_timestamp;
    }

    void insert(string content) {
        if (Active->snapshot_taken) {
            TreeNode nn(total_versions, Active->content+content, "new version created");
            TreeNode* newnode = &nn;
            Active->children.push_back(newnode);
            newnode->Parent = Active;
            Active = newnode;
            total_versions++;

        } else Active->content += content, Active->last_modified=time(nullptr);
        last_modified = Active->last_modified;
    }

    void update(string content) {
        if (Active->snapshot_taken) {
            TreeNode nn(total_versions, content, "new version created");
            TreeNode* newnode = &nn;
            Active->children.push_back(newnode);
            newnode->Parent = Active;
            Active = newnode;
            version_ctrl.insert(total_versions, newnode);
            total_versions++;

        } else Active->content = content, Active->last_modified=time(nullptr);
        last_modified = Active->last_modified;
    }

    void rollback(int v_id) {
        if (v_id==-1) Active = Active->Parent;
        else {
            TreeNode* u = version_ctrl.get(v_id);
            if (u==nullptr) cout<<"No version with this version id"<<'\n';
            else {
                Active = u;
            }
        }
    }

    void snapshot(string m) {
        Active->last_modified = time(nullptr);
        Active->snapshot(m);
        cout<<"Snapshot taken for the file "<<file_name<<" version id "<<Active->version_id;
        snapshotted_versions.push_back({Active->snapshot_timestamp, Active->version_id});
        last_modified = Active->last_modified;
    }

    void read() {
        cout<<Active->content<<'\n';
    }

    void history() {
        sort(snapshotted_versions.begin(), snapshotted_versions.end());
        for (int i=0; i<snapshotted_versions.size(); i++) {
            TreeNode* v = version_ctrl.get(snapshotted_versions[i].second);
            cout<<"File "<<i+1<<'\n';
            cout<<"File Version ID: "<<v->version_id<<'\n';
            cout<<"Snapshot Timestamp: "<<v->snapshot_timestamp<<'\n';
            cout<<"Snapshot message: "<<v->message<<'\n';
            cout<<'\n';
        }
    }
};


// Mainloop
int main() {
    vector<Tree*> ALL_FILES;
    HashMap<string, Tree*> FileMap;
    MaxHeap recent_edits;
    MaxHeap most_versions;
    string inp;
    while (true) {
        cin>>inp;
        vector<string> v;
        for (char c:inp) {
            string a;
            if (c==' ' || c=='\n') {
                v.push_back(a);
                a="";
            } else a+=c;
        }

        if (v[0]!="STOP") {
            if (v[0] == "CREATE") {
                Tree* newfile = new Tree(v[1]);
                ALL_FILES.push_back(newfile);
                FileMap.insert(v[1], newfile);
                recent_edits.insert();
                cout<<"New file "<<v[1]<<" created."<<'\n';
            } else if (v[0] == "READ") {
                cout<<"Here are the contents of the Active version of the file "<<v[1]<<'\n';
                cout<<FileMap.get(v[1])->Active->content<<'\n';
            } else if (v[0] == "HISTORY") {
                cout<<"Here are all the snapshotted versions of the file "<<v[1]<<'\n';
                FileMap.get(v[1])->history();
            } else if (v[0] == "INSERT") {
                FileMap.get(v[1])->insert(v[2]);
                cout<<"Content successfully inserted in the "<<v[1]<<" file."<<'\n';
            } else if (v[0] == "UPDATE") {
                FileMap.get(v[1])->update(v[2]);
                cout<<"Content for "<<v[1]<<" successfully updated."<<'\n';
            } else if (v[0] == "SNAPSHOT") {
                FileMap.get(v[1])->snapshot(v[2]);
            } else if (v[0] == "ROLLBACK") {
                Tree* k = FileMap.get(v[1]);
                k->rollback(((v.size()==3)?stoi(v[2]):-1));
                cout<<"Successfully rolled back to version ID "<<k->Active->version_id<<" for the file "<<v[1]<<'\n';
            } else if (v[0] == "RECENT_FILES") {
                for (Tree* element:ALL_FILES) recent_edits.insert(element->last_modified, element);
                cout<<"Latest Modified files- "<<'\n';
                for (int i=0; i<ALL_FILES.size(); i++) {
                    cout<<recent_edits.removeTop()->file_name<<'\n';
                }
            } else if (v[0] == "BIGGEST_TREES") {
                for (Tree* element:ALL_FILES) most_versions.insert(element->total_versions, element);
                cout<<"Latest Modified files- "<<'\n';
                for (int i=0; i<ALL_FILES.size(); i++) {
                    cout<<most_versions.removeTop()->file_name<<'\n';
                }
            } else if (v[0] == "HELP") {
                cout<<"Following are the available commands with their corresponding syntax"<<'\n';
                cout<<"READ <filename>"<<'\n';
                cout<<"CREATE <filename>"<<'\n';
                cout<<"INSERT <filename> <content>"<<'\n';
                cout<<"UPDATE <filename> <content>"<<'\n';
                cout<<"SNAPSHOT <filename> <message>"<<'\n';
                cout<<"ROLLBACK <filename> <version-id>"<<'\n';
                cout<<"HISTORY <filename>"<<'\n';
                cout<<"RECENT_FILES"<<'\n';
                cout<<"BIGGEST_TREES"<<'\n';
                cout<<"STOP"<<'\n';
                cout<<"HELP"<<'\n';
            } else if (v[0] =="STOP") {

                break;
            }
            else {
                cout<<"INVALID COMMAND. Please Try Again."<<'\n';
                cout<<"You can type 'HELP' to see all the available commands"<<'\n';
            }
        }
    }

    return 0;
}
