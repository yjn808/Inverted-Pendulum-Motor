#include "can.h"





void CAN_Init(void)
{
    //1.开启CAN时钟:CAN、GPIO、AFIO
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;


    //2.重映射PB8、PB9为CAN1的RX、TX
    AFIO->MAPR2 |=AFIO_MAPR_CAN_REMAP_1;
    AFIO->MAPR2 &= ~AFIO_MAPR_CAN_REMAP_0;//与运算将CAN_REMAP_0位置0，其他位保持不变

    //3.GPIO工作模式配置
    //PB8-CAN1_RX-浮空输入 MODE=00 CNF=01
    GPIOB->CRH &= ~(GPIO_CRH_MODE8 | GPIO_CRH_CNF8); //与运算将MODE8和CNF8位置0，其他位保持不变
    GPIOB->CRH |= GPIO_CRH_CNF8_0; //或运算将CNF8的第0位置1，其他位保持不变
    
    //PB9-CAN1-TX-复用推挽输出 MODE=11 CNF=10
    GPIOB->CRH |= GPIO_CRH_MODE9 | GPIO_CRH_CNF9; 
    GPIOB->CRH &= ~GPIO_CRH_CNF9_0;

    //4.CAN初始化配置
    //4.1 进入初始化模式
    CAN1->MCR |=CAN_MCR_INRQ;
    while((CAN1->MSR & CAN_MSR_INAK) == 0); {}//等待进入初始化模式
    
    //4.2退出睡眠模式
    CAN1->MCR &=~CAN_MCR_SLEEP;
    while((CAN1->MSR & CAN_MSR_SLAK) != 0); {}

    //4.3自动离线管理
    CAN1->MCR |= CAN_MCR_ABOM;

    //4.4自动唤醒管理
    CAN1->MCR |= CAN_MCR_AWUM;

    //4.5配置环回静默测试模式
    CAN1->BTR |= CAN_BTR_LBKM | CAN_BTR_SILM;

    //4.6配置位时序
    //4.6.1 配置波特率分频器，36分频，Tq=1us
    CAN1->BTR &= ~CAN_BTR_BRP;
    CAN1->BTR |= 35 << 0;

    //4.6.2 定义配置BS1和BS2的时间长度
    CAN1->BTR &= ~(CAN_BTR_TS1 | CAN_BTR_TS2);
    CAN1->BTR |=(2<<16)|(5<<20);

    //4.6.3再同步跳跃宽度
    CAN1->BTR &= ~CAN_BTR_SJW;
    CAN1->BTR |= (1 << 24);

    //4.7退出初始化模式
    CAN1->MCR &= ~CAN_MCR_INRQ;
    while((CAN1->MSR & CAN_MSR_INAK) != 0);{}
}


//发送CAN报文:标准ID（11位）,数据长度，数据内容（使用发送邮箱0）
void CAN_SendMessage(uint16_t id,uint8_t *data,uint8_t len)
{
    //1.等待发送邮箱为空
    while((CAN1->TSR & CAN_TSR_TME0)==0);{}

    //2.包装要发送的数据帧
    //2.1设置标准ID
    CAN1->sTxMailBox[0].TIR &= ~CAN_TI0R_STID;
    CAN1->sTxMailBox[0].TIR |= (id << 21);

    //2.2设置为标准帧
    CAN1->sTxMailBox[0].TIR &= ~CAN_TI0R_IDE;


    //2.3设置为数据这帧
    CAN1->sTxMailBox[0].TIR &= ~CAN_TI0R_RTR;


    //2.4 设置数据长度
    CAN1->sTxMailBox[0].TDTR &= ~CAN_TDT0R_DLC;
    CAN1->sTxMailBox[0].TDTR |= (len << 0);

    //2.5设置数据
    //先清零寄存器
    CAN1->sTxMailBox[0].TDLR = 0;
    CAN1->sTxMailBox[0].TDHR = 0;

    //循环处理每一个字节数据
    for(uint8_t i=0;i<len;i++)
    {
        //判断前4个字节，放到TDLR寄存器，后4个字节放到TDHR寄存器
        if(i<4)
        {
            CAN1->sTxMailBox[0].TDLR |= (data[i]<<(i*8)); //将数据字节放入TDLR寄存器的相应位置
        }
        else
        {
            CAN1->sTxMailBox[0].TDHR |= (data[i]<<((i-4)*8)); //将数据字节放入TDHR寄存器的相应位置
        }
    }

    //3.请求发送
    CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;

    //4.等待发送完成
    while((CAN1->TSR & CAN_TSR_TXOK0)==0);{}



}


//接收CAN报文：结构体数组。数组长度（从FIFO0中读取）
void CAN_ReceiveMsg(RxMsg *msgArray,uint8_t *msgCount)
{
   //1.获取FIFO0中报文个数，通过指针返回
   *msgCount = (CAN1->RF0R & CAN_RF0R_FMP0) >> 0;

   //2.循环读取每一个报文
   for(uint8_t i=0;i<*msgCount;i++)
   {
    //定义指针指向当前保存报文的数据对象
    RxMsg * msg = &msgArray[i];

    //2.1读取ID
    msg->id = (CAN1->sFIFOMailBox[0].RIR >> 21) & 0x7FF; //标准ID占11位，右移21位后再与0x7FF进行与运算得到ID值

    //2.2读取数据长度
    msg->len = (CAN1->sFIFOMailBox[0].RDTR >> 0) & 0X0F; //数据长度占4位，右移0位后再与0x0F进行与运算得到数据长度值

    //2.3读取数据
    uint32_t dataLow = CAN1->sFIFOMailBox[0].RDLR;
    uint32_t dataHigh = CAN1->sFIFOMailBox[0].RDHR;
    for(uint8_t j=0;j<msg->len;j++)
    {
        if(j<4)
        {
            msg->data[j] = (dataLow >> (j*8)) & 0xFF; //从dataLow寄存器中提取数据字节
        }
        else
        {
            msg->data[j] = (dataHigh >> ((j-4)*8)) & 0xFF; //从dataHigh寄存器中提取数据字节
        }
    }

    //2.4释放FIFO0,出队，读取下一个报文
    CAN1->RF0R |= CAN_RF0R_RFOM0; //设置RFOM0位，释放FIFO0的第一个报文

   }

}



