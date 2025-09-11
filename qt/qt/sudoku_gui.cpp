#include "sudoku_gui.hpp"
#include <QApplication>
#include <QDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStack>
#include <QTime>
#include <QTimer>
#include <QLCDNumber>

//颜色
static const char* BG_FIXED  = "#c0c0c0";   // 题目给定格
static const char* BG_INPUT  = "#ffffff";   // 用户空格
static const char* BG_OK     = "#88ff88";   // 验证正确
static const char* BG_ERROR  = "#ff8888";   // 验证错误
static const char* MICKEY_YELLOW = "#f5e7c9"; // 米黄地板
static const char* MICKEY_BLUE   = "#c9e7f5"; // 浅蓝地板
static const char* MICKEY_BLACK  = "#333333"; // 深灰文字

static int(*g_sol)[9];
static int(*g_init)[9];
static int(*g_play)[9];

//读取和保存
static QString stateFile()
{
    return QDir::home().absoluteFilePath(".sudoku_state.txt");
}
static void saveUserInput()
{
    QFile f(stateFile());
    if(!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream ts(&f);
    for(int r=0;r<9;++r)
        for(int c=0;c<9;++c)
            if(g_init[r][c]==0 && g_play[r][c]!=0)
                ts << r << " " << c << " " << g_play[r][c] << "\n";
}
static QVector<QPair<int,int>> loadUserInput()
{
    QVector<QPair<int,int>> lst;
    QFile f(stateFile());
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) return lst;
    QTextStream ts(&f);
    int r,c,v;
    while(!ts.atEnd()){
        ts >> r >> c >> v;
        if(r<0||r>8||c<0||c>8||v<1||v>9) continue;
        if(g_init[r][c]==0){
            g_play[r][c] = v;
            lst.append({r,c});
        }
    }
    return lst;
}

//重做
struct Snapshot{ int grid[9][9]; };
static QStack<Snapshot> redoStack;

//suduboard的ui与方法
class SudokuBoard : public QWidget{
    Q_OBJECT
public:
    SudokuBoard(QWidget* parent=nullptr): QWidget(parent){
        auto* lay = new QGridLayout(this);
        lay->setSpacing(2);
        for(int r=0;r<9;++r)
            for(int c=0;c<9;++c){
                int val = g_play[r][c];
                if(g_init[r][c]!=0){        // 题目给定
                    le[r][c] = new QLineEdit(QString::number(val),this);
                    le[r][c]->setReadOnly(true);
                    le[r][c]->setStyleSheet(
                        QString("QLineEdit{background:%1;border:1px solid #555;}")
                        .arg(BG_FIXED));
                }else{                      // 用户填写
                    le[r][c] = new QLineEdit(this);
                    le[r][c]->setMaxLength(1);
                    le[r][c]->setValidator(new QIntValidator(1,9,this));
                    le[r][c]->setAlignment(Qt::AlignCenter);
                    le[r][c]->setStyleSheet(
                        QString("QLineEdit{background:%1;border:1px solid #555;}")
                        .arg(BG_INPUT));
                    connect(le[r][c],&QLineEdit::textChanged,[=](const QString&s){
                        g_play[r][c] = s.isEmpty() ? 0 : s.toInt();
                    });
                }
                lay->addWidget(le[r][c],r,c);
            }

        /* 读盘时不再标绿，直接纯白（与 BG_INPUT 一致） */
        for(auto p: loadUserInput()){
            int r=p.first, c=p.second;
            le[r][c]->setText(QString::number(g_play[r][c]));
            // 不再改背景色，保持默认白色
        }
    }

    void verify(){
        bool allOk=true;
        for(int r=0;r<9;++r)
            for(int c=0;c<9;++c){
                if(g_init[r][c]!=0) continue;
                int v=g_play[r][c];
                if(v==0){ allOk=false; continue; }
                if(v==g_sol[r][c]){
                    le[r][c]->setStyleSheet(
                        QString("QLineEdit{background:%1;border:1px solid #555;}")
                        .arg(BG_OK));
                }else{
                    le[r][c]->setStyleSheet(
                        QString("QLineEdit{background:%1;border:2px solid red;}")
                        .arg(BG_ERROR));
                    allOk=false;
                }
            }
        QMessageBox::information(this,"验证结果",
                                 allOk?"全部正确！":"绿色=正确，红色=错误，空白=未填");
    }

    void redo(){
        if(redoStack.isEmpty()) return;
        Snapshot s = redoStack.pop();
        for(int r=0;r<9;++r)
            for(int c=0;c<9;++c){
                g_play[r][c] = s.grid[r][c];
                le[r][c]->setText(g_play[r][c]==0?"":QString::number(g_play[r][c]));
            }
    }
    void pushRedoSnap(){   // 外部调用：保存当前状态到 redo 栈
        Snapshot s;
        for(int r=0;r<9;++r) for(int c=0;c<9;++c) s.grid[r][c]=g_play[r][c];
        redoStack.push(s);
    }
    void updateEntireBoard(){
        for(int r = 0; r < 9; ++r)
        for(int c = 0; c < 9; ++c){
            int v = g_play[r][c];
            le[r][c]->setText(v == 0 ? "" : QString::number(v));
            if(g_init[r][c] != 0) continue;   // 题目给定格不动
            le[r][c]->setStyleSheet(
                QString("QLineEdit{background:%1;border:1px solid #555;}")
                .arg(v == 0 ? BG_INPUT : BG_OK));
        }
    }

private:
    QLineEdit* le[9][9]{};
};

//交互框架
#include <QLCDNumber>
#include <QTimer>
class SudokuDialog : public QDialog{
    Q_OBJECT
public:
    SudokuDialog(QWidget* parent=nullptr):QDialog(parent){
        setWindowTitle("数独 · 米奇黄版");
        resize(420,480);
        // 米黄地板
        setStyleSheet(QString("QDialog{background:%1;}").arg(MICKEY_YELLOW));
        auto* topLay=new QHBoxLayout();
        auto* redoBtn   = new QPushButton("↷ 重做",this);
        auto* restartBtn= new QPushButton("🔄 重启",this);
        lcd=new QLCDNumber(this);
        lcd->setDigitCount(8);
        lcd->setSegmentStyle(QLCDNumber::Flat);
        lcd->display("00:00:00");
        timer=new QTimer(this);
        startTime=QTime::currentTime();
        connect(timer,&QTimer::timeout,[=]{
            int el=startTime.secsTo(QTime::currentTime());
            lcd->display(QTime(0,0).addSecs(el).toString("hh:mm:ss"));
        });
        timer->start(1000);

        redoBtn   ->setStyleSheet(btnStyle(MICKEY_BLACK, MICKEY_YELLOW));
        restartBtn->setStyleSheet(btnStyle(MICKEY_YELLOW, MICKEY_BLACK));
        topLay->addWidget(redoBtn);
        topLay->addWidget(restartBtn);
        topLay->addStretch();
        topLay->addWidget(lcd);

        /* 棋盘 */
        board=new SudokuBoard(this);
        auto* lay=new QVBoxLayout(this);
        lay->addLayout(topLay);
        lay->addWidget(board);

        /* 底部按钮 */
        auto* hLay=new QHBoxLayout();
        auto* verifyBtn=new QPushButton("验证",this);
        auto* closeBtn =new QPushButton("关闭",this);
        verifyBtn->setStyleSheet(btnStyle(MICKEY_YELLOW,"#43a047"));
        closeBtn ->setStyleSheet(btnStyle(MICKEY_YELLOW,MICKEY_BLACK));
        hLay->addWidget(verifyBtn);
        hLay->addWidget(closeBtn);
        lay->addLayout(hLay);

        connect(verifyBtn,&QPushButton::clicked,board,&SudokuBoard::verify);
        connect(closeBtn ,&QPushButton::clicked,this,&QDialog::accept);
        connect(redoBtn,&QPushButton::clicked,[=]{ board->redo(); });
        connect(restartBtn,&QPushButton::clicked,[=]{ onRestart(); });
    }
    void closeEvent(QCloseEvent *e) override
    { saveUserInput(); QDialog::closeEvent(e); }
private:
    SudokuBoard* board;
    QTimer* timer;
    QLCDNumber* lcd;
    QTime startTime;
    QString btnStyle(QString tc,QString bc){
        return QString("QPushButton{color:%1;background:%2;border:none;padding:6px 10px;"
                       "border-radius:4px;font-weight:bold;}").arg(tc,bc);
    }
    void onRestart(){
        for(int r=0;r<9;++r)for(int c=0;c<9;++c) g_play[r][c]=g_init[r][c];
        board->updateEntireBoard();
        while(!redoStack.isEmpty()) redoStack.pop();
        startTime=QTime::currentTime();
    }
};

/* --------------- C 接口 --------------- */
extern "C" void runInteractiveGui(int sol[9][9],
                                  int player[9][9],
                                  int init[9][9])
{
    g_sol  = sol;
    g_play = player;
    g_init = init;
    saveUserInput();
    static int dummy=0;
    static QApplication* app=nullptr;
    if(!QApplication::instance()) app=new QApplication(dummy,nullptr);
    SudokuDialog dlg;
    dlg.exec();
}

#include "sudoku_gui.moc"