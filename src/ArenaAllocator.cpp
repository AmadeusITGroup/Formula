                                                                        
#include "mdw/formula/ArenaAllocator.hpp"
#include <stdlib.h>
#include <stdio.h>

#ifndef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))                                  
#endif                                                                  
                                                                        
#ifndef MIN                                                             
#define MIN(a,b) ((a)<(b) ? (a) : (b))                                  
#endif                                                                  
                                                                        
namespace mdw { namespace formula {

  void ArenaAllocator::DestroyerLink::sinkAll()
  {
    _destroyer.~Destroyer();
    if (_previous)
    {
      _previous->sinkAll();
      _previous = NULL;
    }
  }

  std::string& ArenaAllocator::createString(const char *iStr, size_t iLen)
  {
    return create<std::string>(iStr, iLen);
  }

  /*!                                                                     
    \Call                                                               
    ArenaAllocator::ArenaAllocator                            

    \Description                                                         
    Constructor                                                      

    \Parameters                                                          
iSize: initial block size                                        

\return                                                             
None                                                            

*/                                                                      
  ArenaAllocator::ArenaAllocator(size_t iSize):                 
    _bkSize(MIN(MAX(128, iSize), 8192)), _lastDestroyer(NULL)                                 
  {                                                                       
    uint8_t *aFirstBlock = (uint8_t*)malloc(_bkSize);                 

    _initialBkSize = _bkSize;                                             
    // Init '_blocks'                                                     
    _blocks = (BlockLink*)aFirstBlock;                                
    _blocks->init(NULL);                                                  
    _blocks->putBlock(aFirstBlock);                                       

    // Init the rest                                                      
    _current = (uint8_t*)(_blocks + 1);                                 
    _top = aFirstBlock + _bkSize;                                         
  }                                                                       

  /*!                                                                     
    \Call                                                               
    ArenaAllocator::~ArenaAllocator                           

    \Description                                                         
    Destructor                                                       

    \Parameters                                                          
    None                                                             

    \return                                                             
    None                                                            

*/                                                                      
  ArenaAllocator::~ArenaAllocator()                                 
  {                                                                       
    clean();                                                            
    // The first block is never freed by clean                          
    free(_blocks->_blocks[0]);                                            
  }                                                                       

  /*!                                                                     
    \Call                                                               
    ArenaAllocator::clean                                      

    \Description                                                         
    Releases all blocks except the first one                         

*/                                                                      
  void ArenaAllocator::clean()                                       
  {  
    // First destroy the registered objects before releasing the memory.
    // Destroy them in LIFO order in case of interdependencies!
    if (_lastDestroyer)
    {
      _lastDestroyer->sinkAll();
      _lastDestroyer = NULL;
    }

    int i;                                                                
    BlockLink *aLink;                                                 
    while ((aLink = _blocks->_previous) != NULL)                          
    {                                                                     
      for (i = _blocks->_nbBlocks - 1; i >= 0; i--)                       
        free(_blocks->_blocks[i]);                                        
      _blocks = aLink;                                                    
    }                                                                     
    // We do not free the first block, for efficiency reasons             
    // and to maintain an invariant (_blocks != NULL)                     
    for (i = _blocks->_nbBlocks - 1; i > 0; i--)                          
      free(_blocks->_blocks[i]);                                          
    _blocks->_nbBlocks = 1;                                               
    // Virtually free the first block                                     
    _current = (uint8_t*)(_blocks + 1);

    _top = (uint8_t*)_blocks + _initialBkSize;                                     
    // Reset the block size                                               
    _bkSize = _initialBkSize;                                             
  }                                                                       

  /*!                                                                     
    \Call                                                               
    ArenaAllocator::saveBlock                                    

    \Description                                                         
    Allocates a new block of memory with a minimum size of 'iSize'   

    \Parameters                                                         
iDefault : a block of memory where to allocate the BlockLink \  
by default                                                    
aBlock : the new block whose address should be saved            

\return                                                             
AMD_True if iDefault was used                                   

*/                                                                      
  bool ArenaAllocator::saveBlock(BlockLink *iDefault,          
                                     uint8_t *aBlock)                
  {                                                                       
    bool aRes = false;                                            
    BlockLink *aLink = NULL;                                          
    // Do we need to allocate a new BlockLink to keep track               
    // of the new block?                                                  
    if (_blocks->_nbBlocks >= kBlocksPerLink)                         
    {                                                                     
      // Can it fit in the last block?                                    
      if (_top >= sizeof(BlockLink) + _current)
      {                                                                   
        aLink = (BlockLink*)_current;                                 
        _current += sizeof(BlockLink);                                
      }                                                                   
      else                                                                
      {                                                                   
        // Otherwise put it just after the newly allocated memory         
        aLink = iDefault;                                                 
        aRes = true;                                                  
      }                                                                   

      // Save the block                                                   
      aLink->init(_blocks);                                               
      aLink->putBlock(aBlock);                                            
      _blocks = aLink;                                                    
    }                                                                     
    else                                                                  
    {                                                                     
      _blocks->putBlock(aBlock);                                          
    }                                                                     
    return aRes;                                                          
  }                                                                       
  /*!                                                                     
    \Call                                                               
    ArenaAllocator::getNewBlock                                  

    \Description                                                         
    Allocates a new block of memory with a minimum size of 'iSize'   

*/                                                                      
  uint8_t *ArenaAllocator::getNewBlock(size_t iSize)             
  {                                                                       
    uint8_t *aResult;                                                   
    if (iSize > (_bkSize - sizeof(BlockLink))/2)                        
    {                                                                     
      // In case of a request for a huge block, we just malloc it directly
      aResult = (uint8_t*)malloc(iSize + sizeof(BlockLink));        
      saveBlock((BlockLink*)(aResult + iSize), aResult);              
    }                                                                     
    else                                                                  
    {                                                                     
      uint8_t *aBlock;                                                  
      // We allocate a new block and intend to put other things into it   
      _bkSize = MIN(_bkSize * 2, 8192);                                   
      aBlock = (uint8_t*)malloc(_bkSize);                               
      aResult = aBlock;                                                   
      if (saveBlock((BlockLink*)aBlock, aBlock))                      
        aResult += sizeof(BlockLink);                                 
      _current = aResult + iSize;
      MDW_ALIGN(_current);
      _top = aBlock + _bkSize;                                            
    }                                                                     
    return aResult;                                                       
  }                                                                       

}}
