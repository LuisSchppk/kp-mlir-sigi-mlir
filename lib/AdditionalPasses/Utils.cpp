#include "Utils.h"

using namespace mlir;
bool isBeforeInOp(Operation* first, Operation* second)
{
    assert(first && "First is null.");
    assert(second && "Second is null.");
    auto firstBlock = first->getBlock();
    auto secondBlock = second->getBlock();
    bool isBefore = false;
    if (firstBlock == secondBlock) {
        isBefore = first->isBeforeInBlock(second);
    } else {
        llvm::SmallVector<Block*> queue{firstBlock};
        int i = 0;
        int size = queue.size();
        while (i < size && !isBefore) {
            auto current = queue[i++];
            isBefore = current == secondBlock;
            for (auto succ : current->getSuccessors())
                if (!isBefore && !llvm::is_contained(queue, succ)) queue.push_back(succ);
            size = queue.size();
        }
    }
    return isBefore;
}