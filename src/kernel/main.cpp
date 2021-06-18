namespace onix
{
    namespace kernel
    {
        extern "C"
        {
            int main()
            {
                char *video = (char *)(0xb8000);
                *video = 'K';
                return 0;
            }
        }
    }
};