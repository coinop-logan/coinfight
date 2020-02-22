#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include "vchpack.h"
#include "engine.h"
#include "cmds.h"

using namespace std;
using namespace boost::asio::ip;

int main()
{
    FrameCmdsPacket fp(10, vector<Cmd>(2));

    vch data;
    fp.pack(&data);

    cout << "packed" << endl;

    vchIter dBegin = data.begin();
    FrameCmdsPacket fp2(&dBegin);

    cout << "unpacked: " << fp2.frame << "," << fp2.cmds.size() << endl;

    return 0;
}