#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <unordered_map>
/* --------------------------------
          MenuResourceBase
---------------------------------- */
struct MenuResourceBase {
    MenuResourceBase() {
        std::cout << "  MenuResourceBase::MenuResourceBase()\n";
    }

    virtual void load( std::string filename ) = 0;
    virtual bool is_loaded() = 0;

    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void cleanup() = 0;

    // Note: non-virtual destructor is OK here
    ~MenuResourceBase() {
        std::cout << "  MenuResourceBase::~MenuResourceBase()\n";
    }
};

/* --------------------------------
            ScreenWriter
---------------------------------- */
class ScreenWriter: public MenuResourceBase {

    bool loaded_state;

public:
    ScreenWriter() : loaded_state(false) {
        std::cout << "  ScreenWriter::ScreenWriter()\n";
    }

    ~ScreenWriter() {
        //cleanup();
        std::cout << "  ScreenWriter::~ScreenWriter()\n";
    }

    /* Virtual Defines */
    void load( std::string filename ) {
        if (!loaded_state) {
            //std::cout << "      ScreenWriter Loading Stuff!: " << filename << std::endl;
            float sum(0.0f);
            for (unsigned i = 0; i < 500000000; ++i) {
                sum +=1.0f;
            }
            //std::cout << "      ScreenWriter Finished" << std::endl;

            loaded_state=true; // Change to loaded state
        }
    };

    /* Checks if it is loaded */
    bool is_loaded() {
        return loaded_state;
    };

    /* Update the class */
    void update() {};

    /* Draw things */
    void draw() {};

    /* Cleanup the class */
    void cleanup() {};
};

/* --------------------------------
            ScreenWriter
---------------------------------- */
class ImageLoader: public MenuResourceBase {

    bool loaded_state;

public:
    ImageLoader() : loaded_state(false) {
        std::cout << "  ImageLoader::ImageLoader()\n";
    }

    ~ImageLoader() {
        cleanup();
        std::cout << "  ImageLoader::~ImageLoader()\n";
    }

    /* Virtual Defines */
    void load( std::string filename ) {
        if (!loaded_state) {
            //std::cout << "      ImageLoader Loading Stuff!: " << filename << std::endl;
            //std::this_thread::sleep_for(std::chrono::seconds(5));

            float sum(0.0f);
            for (unsigned i = 0; i < 500000000; ++i) {
                sum +=1.0f;
            }

            //std::cout << "      ImageLoader Finished" << std::endl;

            loaded_state=true; // Change to loaded state
        }
    };

    /* Checks if it is loaded */
    bool is_loaded() {
        return loaded_state;
    };

    /* Update the class */
    void update() {};

    /* Draw things */
    void draw() {};

    /* Cleanup the class */
    void cleanup() {};
};

/* --------------------------------
          thr() Function
---------------------------------- */
void thr_loader(std::shared_ptr<MenuResourceBase> p,std::string filename) {
    // shared use_count is incremented
    {
        //static std::mutex load_mutex;
        //std::lock_guard<std::mutex> lk(load_mutex);
        p.get()->load(filename);
    }
}

typedef std::unordered_map< std::string,std::shared_ptr<MenuResourceBase> > resource_umap;

class MenuResourceManager {

    resource_umap umap;
    std::vector<std::string> resourcestack;

    public:
    /* Constructor */
    MenuResourceManager () {};

    /* Destructor */
    ~MenuResourceManager () {};

    /* Function for Requesting Resources */
    std::shared_ptr<MenuResourceBase> requestResource( std::string ident ) {
        unsigned n( ident.find_first_of(":") );

        std::string type = ident.substr(0,n);
        resource_umap::iterator it = umap.find(ident);

        if ( it == umap.end() ) {

            if        ( type == std::string("screenwriter") ) {
                umap.emplace(ident,std::make_shared<ScreenWriter>());
            } else if ( type == std::string("imageloader") ) {
                umap.emplace(ident,std::make_shared<ImageLoader >());
            }

            resourcestack.push_back(ident);

            it = umap.find(ident);
            return (*it).second;
        }

        return (*it).second;
    };

    /* This function loads all resources in the stack */
    void manageResources () {

        resource_umap::iterator it;

        while (!resourcestack.empty()) {
            it = umap.find(resourcestack.back());
            resourcestack.pop_back();
            std::thread (thr_loader,(*it).second,(*it).first.substr((*it).first.find_first_of(":")+1)).detach();
        }

        it = umap.begin();
        for (;it != umap.end();++it) {
            if ( (*it).second.use_count() < 2 ) {
                (*it).second.reset();
                umap.erase(it);
            }
        }
    };

    /* Print loaded or not */
    void printLoad() {

        for (auto&& i : umap) {
            if (i.second.get()->is_loaded()) {std::cout << i.first << " LOADED!" << std::endl;} else {std::cout << i.first << " WAITING!" << std::endl;};
        }

        std::cout << std::endl;
    };
};

/* --------------------------------
               Main
---------------------------------- */
int main() {
    std::string fname1("screenwriter:fontone.dat");
    //std::string fname2("screenwriter:fonttwo.dat");
    //std::string fname3("imageloader:imageone.dat");
    //std::string fname4("imageloader:imagetwo.dat");
    //std::string fname5("imageloader:imagethree.dat");

    MenuResourceManager mrm;
    std::shared_ptr<MenuResourceBase> p = mrm.requestResource(fname1);

    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";

    std::cout << "USE: " << p.use_count() << std::endl;

    while (true) { // Simulate a graphix engine loop
        mrm.manageResources();

        std::cout << "Hi, I'm the main thread, waiting for my children threads to finish!\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //mrm.printLoad();

        if (p)
            p.reset();
    }

    std::cout << "Hi, I'm Main after the threads finished loading!\n";

    //p1.reset();
    std::cout << "All threads completed, the last one deleted Derived Classes\n";
}
