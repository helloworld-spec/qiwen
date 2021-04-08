### 本地创建仓库

```
git init
```

### 拉取远端项目

```
git clone [项目仓库地址] -b(指定分支名) [分支名]
```

### 指定要缓存到本地版本管理器的文件

```
git add [文件或目录]
```

### 把修改的文件推送到本地版本管理器

```
git commit -m "所做修改的描述"
```

### 把本地仓库推送到远端

```
git push origin [分支名 ]
```

### 查看文件的状态

```
git status
可指定文件
git status [文件名]
```

### 查看所有的版本信息 

```
git log
```

### 修改文件后和服务器的比对，做了哪些修改

```
git diff [文件]
```

### 创建分支

```
git branch [分支名]
```

### 切换分支

```
git checkout [分支名]
```

### add以后撤销

```
git reset [不需要的文件]
```

### commit后回退到某一个版本

```
git reset --hard [版本号]
```

### push后发现push错了

```
先把本地的回退到想要push的版本
git reset --hard [版本号]
再add、commit、push.push时候因为版本低，可以
git push origin [分支名] --force
```

### git push冲突

```
git pull origin develop
手动解决冲突
```

### git 回退到指定commit

```c
git reset --hard [commit_id]
```

### 把一个分支的提交提交到另一个分支

```
如：把分支a的提交提交到分支b
在分支b执行命令
git cherry-pick [branch_a_commit_id]
```

### 追加一个commit到上一个commit

```
git commit --amend
```

```
git reset --hard HEAD~2
```

```
git tag [message]
git push --tag
```

```
git checkout -b [本地分支名] origin/[远程分支名]
```

```
git branch -D [要删除的本地分支名]
```

### git放弃本地文件修改

```
1、未使用add缓存文件
git checkout -- [file]
放弃所有还没有加入缓存区（就是git ad）的修改，内容修改和整个文件删除
此命令不会删除新建的文件，因为新建的文件还没加入git管理系统中，对git来说是未知的，手动删除就好
2、已使用git add 缓存代码，未使用git commit
git reset HEAD [filename]	//放弃某个文件的add操作
此命令用来清楚git对于文件修改的缓存。相当于撤销git add命令的工作，在使用本命令后，本地的修改 并不会消失，而是回到第一步：1、未使用git add缓存代码，继续使用
git checkout -- [file]
就可以放弃本地修改
3、已经用git commit提交了代码
使用git reset --hard HEAD^来回退到上一次commit的状态
git reset --hard HEAD^
或者回退到任意版本git reset --hard commit id,使用git log命令查看git提交历史和commit id


```

