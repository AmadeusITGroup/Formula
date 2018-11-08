#pragma once

#include <stdio.h>
#include <string>
#include <stdint.h>

#define kBlocksPerLink 6
#ifndef	ALIGN_SIZE
#define	ALIGN_SIZE	sizeof(char*)
#endif
#define MDW_ALIGN(p) (p)=(uint8_t*)((((size_t)(p) - 1)/ALIGN_SIZE + 1)*ALIGN_SIZE)

namespace mdw { namespace formula {

  /**
   * A class that allocates memory in a buffer and frees all the memory at once.
   * Especially efficient for allocating lots of small objects
   * \ingroup tbxdatamgr
   */
  class ArenaAllocator
  {
    class Destroyer {
    public:
      virtual ~Destroyer() {}
    };

    template <class T> class TypedDestroyer: public Destroyer
    {
      T *_object;
    public:
      TypedDestroyer(T& ioObject):
        _object(&ioObject)
      {}

      ~TypedDestroyer()
      {
        _object->~T();
      }
    };

    template <class T> class TypedDeleter: public Destroyer
    {
      T *_object;
    public:
      TypedDeleter(T& ioObject):
        _object(&ioObject)
      {}

      ~TypedDeleter()
      {
        delete _object;
      }
    };

    class DestroyerLink {
      Destroyer& _destroyer;
      DestroyerLink *_previous;

    public:
      DestroyerLink(Destroyer& ioTarget, DestroyerLink *ioPrevious):
        _destroyer(ioTarget), _previous(ioPrevious)
      {
      }

      void sinkAll();
    };
  
  public:

    // ioObject will be destroyed when the FastAllocator memory is released
    template <class T> void registerForDestruction(T& ioObject)
    {
      void *aBuffer = allocate(sizeof(TypedDestroyer<T>));
      TypedDestroyer<T> *aDestroyer = new (aBuffer) TypedDestroyer<T>(ioObject);
      aBuffer = allocate(sizeof(DestroyerLink));
      DestroyerLink *aLink = new (aBuffer) DestroyerLink(*aDestroyer, _lastDestroyer);
      _lastDestroyer = aLink;
    }

    // ioObject will be deleted when the ArenaAllocator is destroyed
    template <class T> void registerForDeletion(T& ioObject)
    {
      void *aBuffer = allocate(sizeof(TypedDeleter<T>));
      TypedDeleter<T> *aDestroyer = new (aBuffer) TypedDeleter<T>(ioObject);
      aBuffer = allocate(sizeof(DestroyerLink));
      DestroyerLink *aLink = new (aBuffer) DestroyerLink(*aDestroyer, _lastDestroyer);
      _lastDestroyer = aLink;
    }

    std::string& createString(const char *iStr, size_t iLen);

    template <class E> E& create()
    {
      E& aNewExpr = *(new (allocate(sizeof(E))) E());
      registerForDestruction(aNewExpr);
      return aNewExpr;
    }

    template <class E, class InputT> E& create(InputT& iValue1)
    {
      E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1));
      registerForDestruction(aNewExpr);
      return aNewExpr;
    }

    template <class E, class InputT1, class InputT2>
      E& create(InputT1& iValue1, InputT2& iValue2)
      {
        E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1, iValue2));
        registerForDestruction(aNewExpr);
        return aNewExpr;
      }

    template <class E, class InputT1, class InputT2, class InputT3>
      E& create(InputT1& iValue1, InputT2& iValue2, InputT3& iValue3)
      {
        E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1, iValue2, iValue3));
        registerForDestruction(aNewExpr);
        return aNewExpr;
      }

    template <class E, class InputT1, class InputT2, class InputT3, class InputT4>
      E& create(InputT1& iValue1, InputT2& iValue2,
                InputT3& iValue3, InputT4& iValue4)
      {
        E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1, iValue2, iValue3, iValue4));
        registerForDestruction(aNewExpr);
        return aNewExpr;
      }

    template <class E, class InputT1, class InputT2, class InputT3, class InputT4, class InputT5>
      E& create(InputT1& iValue1, InputT2& iValue2,
                InputT3& iValue3, InputT4& iValue4,
                InputT5& iValue5)
      {
        E& aNewExpr =
          *(new (allocate(sizeof(E))) E(iValue1, iValue2, iValue3, iValue4, iValue5));
        registerForDestruction(aNewExpr);
        return aNewExpr;
      }

    template <class E> E& createUntracked()
    {
      E& aNewExpr = *(new (allocate(sizeof(E))) E());
      return aNewExpr;
    }

    template <class E, class InputT> E& createUntracked(InputT& iValue1)
    {
      E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1));
      return aNewExpr;
    }

    template <class E, class InputT1, class InputT2>
      E& createUntracked(InputT1& iValue1, InputT2& iValue2)
      {
        E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1, iValue2));
        return aNewExpr;
      }

    template <class E, class InputT1, class InputT2, class InputT3>
      E& createUntracked(InputT1& iValue1, InputT2& iValue2, InputT3& iValue3)
      {
        E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1, iValue2, iValue3));
        return aNewExpr;
      }

    template <class E, class InputT1, class InputT2, class InputT3, class InputT4>
      E& createUntracked(InputT1& iValue1, InputT2& iValue2,
                InputT3& iValue3, InputT4& iValue4)
      {
        E& aNewExpr = *(new (allocate(sizeof(E))) E(iValue1, iValue2, iValue3, iValue4));
        return aNewExpr;
      }

    template <class E, class InputT1, class InputT2, class InputT3, class InputT4, class InputT5>
      E& createUntracked(InputT1& iValue1, InputT2& iValue2,
                InputT3& iValue3, InputT4& iValue4,
                InputT5& iValue5)
      {
        E& aNewExpr =
          *(new (allocate(sizeof(E))) E(iValue1, iValue2, iValue3, iValue4, iValue5));
        return aNewExpr;
      }

    //! Constructor that lets the user decide the initial block size
    ArenaAllocator(size_t iSize = 512);

    // Destructor
    ~ArenaAllocator();

    // Returns a memory block of 'iSize' bytes
    void *allocate(size_t iSize)
    {
      if (_current + iSize >= _current && _current + iSize <= _top)
      {
        uint8_t *aResult = _current;
        _current += iSize;
        MDW_ALIGN(_current);
        return aResult;
      }
      else
      {
        return getNewBlock(iSize);
      }
    }

    // Releases all allocated objects
    void clean();

  protected:
    //! Inner class that remembers the allocated blocks and can be linked
    class BlockLink
    {
    public:
      void init(BlockLink *iPrev)
      {
        _nbBlocks = 0;
        _previous = iPrev;
      }

      void putBlock(uint8_t *iBlock)
      {
        _blocks[_nbBlocks++] = iBlock;
      }

      uint16_t     _nbBlocks;
      uint16_t     _spare;
      uint8_t     *_blocks[kBlocksPerLink];
      BlockLink     *_previous;
    };

    //! Current watermark
    uint8_t *_current;

    //! Top position in the allocated block
    uint8_t *_top;

    //! Current block size
    size_t _bkSize, _initialBkSize;

    //! Pointer to the blocks of memory
    BlockLink *_blocks;

    uint8_t *getNewBlock(size_t iSize);

    bool saveBlock(BlockLink *, uint8_t *);

    // LIFO to destroy registered objects
    DestroyerLink *_lastDestroyer;
  private:
    ArenaAllocator(const ArenaAllocator&);
    ArenaAllocator& operator = (const ArenaAllocator&);

  };

}}

