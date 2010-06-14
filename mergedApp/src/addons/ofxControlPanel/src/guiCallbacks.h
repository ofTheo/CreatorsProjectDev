#ifndef __GUI_CALLBACK_H
#define __GUI_CALLBACK_H

class BaseCallback
{
    public:
        virtual void Execute(float param1, float param2, int param_mode, int callback_id) const = 0;
};


template <class cInstance>
class ParameterCallback : public BaseCallback
{
    public:
        ParameterCallback()    // constructor
        {
            pFunction = 0;
        }

        typedef void (cInstance::*tFunction)(float param1, float param2, int param_mode, int callback_id);

        virtual void Execute(float param1, float param2, int param_mode, int callback_id) const
        {
            if (pFunction)
                (cInst->*pFunction)(param1, param2, param_mode, callback_id);
            else
                printf("ERROR : the callback function has not been defined !!!!");
        }

        void SetCallback (cInstance  *cInstancePointer,
                          tFunction   pFunctionPointer)
        {
            cInst     = cInstancePointer;
            pFunction = pFunctionPointer;
        }

    private:
        cInstance  *cInst;
        tFunction  pFunction;
};

#endif
